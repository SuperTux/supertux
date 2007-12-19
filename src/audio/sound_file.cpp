//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

/** Used SDL_mixer and glest source as reference */
#include <config.h>

#include "sound_file.hpp"

#include <stdio.h>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <assert.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <unison/vfs/sdl/Utils.hpp>
#include <unison/vfs/FileSystem.hpp>
#include "SDL.h"
#include "log.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"

class WavSoundFile : public SoundFile
{
public:
  WavSoundFile(SDL_RWops* file);
  ~WavSoundFile();

  size_t read(void* buffer, size_t buffer_size);
  void reset();

private:
  SDL_RWops* file;

  int datastart;
};

WavSoundFile::WavSoundFile(SDL_RWops* file)
{
  this->file = file;

  char magic[4];
  if(SDL_RWread(file, magic, sizeof(magic), 1) != 1)
    throw std::runtime_error("Couldn't read file magic (not a wave file)");
  if(strncmp(magic, "RIFF", 4) != 0) {
    log_debug << "MAGIC: " << magic << std::endl;
    throw std::runtime_error("file is not a RIFF wav file");
  }

  Uint32 wavelen = SDL_ReadLE32(file);
  (void) wavelen;

  if(SDL_RWread(file, magic, sizeof(magic), 1) != 1)
    throw std::runtime_error("Couldn't read chunk header (not a wav file?)");
  if(strncmp(magic, "WAVE", 4) != 0)
    throw std::runtime_error("file is not a valid RIFF/WAVE file");

  char chunkmagic[4];
  Uint32 chunklen;

  // search audio data format chunk
  do {
    if(SDL_RWread(file, chunkmagic, sizeof(chunkmagic), 1) != 1)
      throw std::runtime_error("EOF while searching format chunk");
    chunklen = SDL_ReadLE32(file);

    if(strncmp(chunkmagic, "fmt ", 4) == 0)
      break;

    if(strncmp(chunkmagic, "fact", 4) == 0
        || strncmp(chunkmagic, "LIST", 4) == 0) {
      // skip chunk
      if(SDL_RWseek(file, chunklen, RW_SEEK_CUR) == 0)
        throw std::runtime_error("EOF while searching fmt chunk");
    } else {
      throw std::runtime_error("complex WAVE files not supported");
    }
  } while(true);

  if(chunklen < 16)
    throw std::runtime_error("Format chunk too short");

  // parse format
  Uint16 encoding = SDL_ReadLE16(file);
  if(encoding != 1)
    throw std::runtime_error("only PCM encoding supported");
  channels = SDL_ReadLE16(file);
  rate = SDL_ReadLE32(file);
  Uint32 byterate = SDL_ReadLE32(file);
  (void) byterate;
  Uint16 blockalign = SDL_ReadLE16(file);
  (void) blockalign;
  bits_per_sample = SDL_ReadLE16(file);

  if(chunklen > 16) {
    if(SDL_RWseek(file, (chunklen-16), RW_SEEK_CUR) == 0)
      throw std::runtime_error("EOF while reading reast of format chunk");
  }

  // set file offset to DATA chunk data
  do {
    if(SDL_RWread(file, chunkmagic, sizeof(chunkmagic), 1) != 1)
      throw std::runtime_error("EOF while searching data chunk");
    chunklen = SDL_ReadLE32(file);

    if(strncmp(chunkmagic, "data", 4) == 0)
      break;

    // skip chunk
    if(SDL_RWseek(file, chunklen, RW_SEEK_CUR) == 0)
      throw std::runtime_error("EOF while searching fmt chunk");
  } while(true);

  datastart = SDL_RWtell(file);
  size = static_cast<size_t> (chunklen);
}

WavSoundFile::~WavSoundFile()
{
  SDL_RWclose(file);
}

void
WavSoundFile::reset()
{
  if(SDL_RWseek(file, datastart, RW_SEEK_SET) == 0)
    throw std::runtime_error("Couldn't seek to data start");
}

size_t
WavSoundFile::read(void* buffer, size_t buffer_size)
{
  int end = datastart + size;
  int cur = SDL_RWtell(file);
  if(cur >= end)
    return 0;

  size_t readsize = std::min(static_cast<size_t> (end - cur), buffer_size);
  if(SDL_RWread(file, buffer, readsize, 1) != 1)
    throw std::runtime_error("read error while reading samples");

#ifdef WORDS_BIGENDIAN
  if (bits_per_sample != 16)
    return readsize;
  char *tmp = (char*)buffer;

  size_t i;
  char c;
  for (i = 0; i < readsize / 2; i++)
  {
    c          = tmp[2*i];
    tmp[2*i]   = tmp[2*i+1];
    tmp[2*i+1] = c;
  }

  buffer = tmp;
#endif

  return readsize;
}

//---------------------------------------------------------------------------

class OggSoundFile : public SoundFile
{
public:
  OggSoundFile(SDL_RWops* file, double loop_begin, double loop_at);
  ~OggSoundFile();

  size_t read(void* buffer, size_t buffer_size);
  void reset();

private:
  static size_t cb_read(void* ptr, size_t size, size_t nmemb, void* source);
  static int cb_seek(void* source, ogg_int64_t offset, int whence);
  static int cb_close(void* source);
  static long cb_tell(void* source);

  SDL_RWops*   file;
  OggVorbis_File vorbis_file;
  ogg_int64_t    loop_begin;
  ogg_int64_t    loop_at;
  size_t         normal_buffer_loop;
};

OggSoundFile::OggSoundFile(SDL_RWops* file, double loop_begin, double loop_at)
{
  this->file = file;

  ov_callbacks callbacks = { cb_read, cb_seek, cb_close, cb_tell };
  ov_open_callbacks(file, &vorbis_file, 0, 0, callbacks);

  vorbis_info* vi = ov_info(&vorbis_file, -1);

  channels        = vi->channels;
  rate            = vi->rate;
  bits_per_sample = 16;
  size            = static_cast<size_t> (ov_pcm_total(&vorbis_file, -1) * 2);

  double sample_len    = 1.0f / rate;
  double samples_begin = loop_begin / sample_len;
  double sample_loop   = loop_at / sample_len;

  this->loop_begin     = (ogg_int64_t) samples_begin;
  if(loop_begin < 0) {
    this->loop_at = (ogg_int64_t) -1;
  } else {
    this->loop_at = (ogg_int64_t) sample_loop;
  }
}

OggSoundFile::~OggSoundFile()
{
  ov_clear(&vorbis_file);
}

size_t
OggSoundFile::read(void* _buffer, size_t buffer_size)
{
  char*  buffer         = reinterpret_cast<char*> (_buffer);
  int    section        = 0;
  size_t totalBytesRead = 0;

  while(buffer_size>0) {
#ifdef WORDS_BIGENDIAN
    int bigendian = 1;
#else
    int bigendian = 0;
#endif

    size_t bytes_to_read    = buffer_size;
    if(loop_at > 0) {
      size_t      bytes_per_sample       = 2;
      ogg_int64_t time                   = ov_pcm_tell(&vorbis_file);
      ogg_int64_t samples_left_till_loop = loop_at - time;
      ogg_int64_t bytes_left_till_loop
        = samples_left_till_loop * bytes_per_sample;
      if(bytes_left_till_loop <= 4)
        break;

      if(bytes_left_till_loop < (ogg_int64_t) bytes_to_read) {
        bytes_to_read    = (size_t) bytes_left_till_loop;
      }
    }

    long bytesRead
      = ov_read(&vorbis_file, buffer, bytes_to_read, bigendian,
          2, 1, &section);
    if(bytesRead == 0) {
      break;
    }
    buffer_size    -= bytesRead;
    buffer         += bytesRead;
    totalBytesRead += bytesRead;
  }

  return totalBytesRead;
}

void
OggSoundFile::reset()
{
  ov_pcm_seek(&vorbis_file, loop_begin);
}

size_t
OggSoundFile::cb_read(void* ptr, size_t size, size_t nmemb, void* source)
{
  SDL_RWops* file = reinterpret_cast<SDL_RWops*> (source);

  int res = SDL_RWread(file, ptr, size, nmemb);
  if(res <= 0)
    return 0;

  return static_cast<size_t> (res);
}

int
OggSoundFile::cb_seek(void* source, ogg_int64_t offset, int whence)
{
  SDL_RWops* file = reinterpret_cast<SDL_RWops*> (source);

  switch(whence) {
    case SEEK_SET:
      if(SDL_RWseek(file, offset, RW_SEEK_SET) == 0)
        return -1;
      break;
    case SEEK_CUR:
      if(SDL_RWseek(file, offset, RW_SEEK_CUR) == 0)
        return -1;
      break;
    case SEEK_END:
      if(SDL_RWseek(file, offset, RW_SEEK_END) == 0)
        return -1;
      break;
    default:
#ifdef DEBUG
      assert(false);
#else
      return -1;
#endif
  }
  return 0;
}

int
OggSoundFile::cb_close(void* source)
{
  SDL_RWops* file = reinterpret_cast<SDL_RWops*> (source);
  SDL_RWclose(file);
  return 0;
}

long
OggSoundFile::cb_tell(void* source)
{
  SDL_RWops* file = reinterpret_cast<SDL_RWops*> (source);
  return SDL_RWtell(file);
}

//---------------------------------------------------------------------------

SoundFile* load_music_file(const std::string& filename)
{
  lisp::Parser parser(false);
  const lisp::Lisp* root = parser.parse(filename);
  const lisp::Lisp* music = root->get_lisp("supertux-music");
  if(music == NULL)
    throw std::runtime_error("file is not a supertux-music file.");

  std::string raw_music_file;
  float loop_begin = 0;
  float loop_at    = -1;

  music->get("file", raw_music_file);
  music->get("loop-begin", loop_begin);
  music->get("loop-at", loop_at);
  
  if(loop_begin < 0) {
    throw std::runtime_error("can't loop from negative value");
  }

  std::string basedir = Unison::VFS::FileSystem::dirname(filename);
  raw_music_file = basedir + "/" + raw_music_file;

  SDL_RWops* file = Unison::VFS::SDL::Utils::open_physfs_in(raw_music_file);

  return new OggSoundFile(file, loop_begin, loop_at);
}

SoundFile* load_sound_file(const std::string& filename)
{
  if(filename.length() > 6
      && filename.compare(filename.length()-6, 6, ".music") == 0) {
    return load_music_file(filename);
  }

  SDL_RWops* file = Unison::VFS::SDL::Utils::open_physfs_in(filename);

  try {
    char magic[4];
    if(SDL_RWread(file, magic, sizeof(magic), 1) != 1)
      throw std::runtime_error("Couldn't read magic, file too short");
    SDL_RWseek(file, 0, RW_SEEK_SET);
    if(strncmp(magic, "RIFF", 4) == 0)
      return new WavSoundFile(file);
    else if(strncmp(magic, "OggS", 4) == 0)
      return new OggSoundFile(file, 0, -1);
    else
      throw std::runtime_error("Unknown file format");
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Couldn't read '" << filename << "': " << e.what();
    throw std::runtime_error(msg.str());
  }
}
