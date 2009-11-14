//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

/** Used SDL_mixer and glest source as reference */
#include <config.h>

#include "sound_file.hpp"

#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <assert.h>

#include <physfs.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "util/log.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "util/file_system.hpp"

class SoundError : public std::exception
{
public:
  SoundError(const std::string& message) throw();
  virtual ~SoundError() throw();

  const char* what() const throw();
private:
  std::string message;
};

SoundError::SoundError(const std::string& message) throw()
{
  this->message = message;
}

SoundError::~SoundError() throw()
{}

const char*
SoundError::what() const throw()
{
  return message.c_str();
}

class WavSoundFile : public SoundFile
{
public:
  WavSoundFile(PHYSFS_file* file);
  ~WavSoundFile();

  size_t read(void* buffer, size_t buffer_size);
  void reset();

private:
  PHYSFS_file* file;

  PHYSFS_sint64 datastart;
};

static inline uint32_t read32LE(PHYSFS_file* file)
{
  uint32_t result;
  if(PHYSFS_readULE32(file, &result) == 0)
    throw SoundError("file too short");

  return result;
}

static inline uint16_t read16LE(PHYSFS_file* file)
{
  uint16_t result;
  if(PHYSFS_readULE16(file, &result) == 0)
    throw SoundError("file too short");

  return result;
}

WavSoundFile::WavSoundFile(PHYSFS_file* file)
{
  this->file = file;

  char magic[4];
  if(PHYSFS_read(file, magic, sizeof(magic), 1) != 1)
    throw SoundError("Couldn't read file magic (not a wave file)");
  if(strncmp(magic, "RIFF", 4) != 0) {
    log_debug << "MAGIC: " << magic << std::endl;
    throw SoundError("file is not a RIFF wav file");
  }

  uint32_t wavelen = read32LE(file);
  (void) wavelen;

  if(PHYSFS_read(file, magic, sizeof(magic), 1) != 1)
    throw SoundError("Couldn't read chunk header (not a wav file?)");
  if(strncmp(magic, "WAVE", 4) != 0)
    throw SoundError("file is not a valid RIFF/WAVE file");

  char chunkmagic[4];
  uint32_t chunklen;

  // search audio data format chunk
  do {
    if(PHYSFS_read(file, chunkmagic, sizeof(chunkmagic), 1) != 1)
      throw SoundError("EOF while searching format chunk");
    chunklen = read32LE(file);

    if(strncmp(chunkmagic, "fmt ", 4) == 0)
      break;

    if(strncmp(chunkmagic, "fact", 4) == 0
        || strncmp(chunkmagic, "LIST", 4) == 0) {
      // skip chunk
      if(PHYSFS_seek(file, PHYSFS_tell(file) + chunklen) == 0)
        throw SoundError("EOF while searching fmt chunk");
    } else {
      throw SoundError("complex WAVE files not supported");
    }
  } while(true);

  if(chunklen < 16)
    throw SoundError("Format chunk too short");

  // parse format
  uint16_t encoding = read16LE(file);
  if(encoding != 1)
    throw SoundError("only PCM encoding supported");
  channels = read16LE(file);
  rate = read32LE(file);
  uint32_t byterate = read32LE(file);
  (void) byterate;
  uint16_t blockalign = read16LE(file);
  (void) blockalign;
  bits_per_sample = read16LE(file);

  if(chunklen > 16) {
    if(PHYSFS_seek(file, PHYSFS_tell(file) + (chunklen-16)) == 0)
      throw SoundError("EOF while reading rest of format chunk");
  }

  // set file offset to DATA chunk data
  do {
    if(PHYSFS_read(file, chunkmagic, sizeof(chunkmagic), 1) != 1)
      throw SoundError("EOF while searching data chunk");
    chunklen = read32LE(file);

    if(strncmp(chunkmagic, "data", 4) == 0)
      break;

    // skip chunk
    if(PHYSFS_seek(file, PHYSFS_tell(file) + chunklen) == 0)
      throw SoundError("EOF while searching fmt chunk");
  } while(true);

  datastart = PHYSFS_tell(file);
  size = static_cast<size_t> (chunklen);
}

WavSoundFile::~WavSoundFile()
{
  PHYSFS_close(file);
}

void
WavSoundFile::reset()
{
  if(PHYSFS_seek(file, datastart) == 0)
    throw SoundError("Couldn't seek to data start");
}

size_t
WavSoundFile::read(void* buffer, size_t buffer_size)
{
  PHYSFS_sint64 end = datastart + size;
  PHYSFS_sint64 cur = PHYSFS_tell(file);
  if(cur >= end)
    return 0;

  size_t readsize = std::min(static_cast<size_t> (end - cur), buffer_size);
  if(PHYSFS_read(file, buffer, readsize, 1) != 1)
    throw SoundError("read error while reading samples");

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
  OggSoundFile(PHYSFS_file* file, double loop_begin, double loop_at);
  ~OggSoundFile();

  size_t read(void* buffer, size_t buffer_size);
  void reset();

private:
  static size_t cb_read(void* ptr, size_t size, size_t nmemb, void* source);
  static int cb_seek(void* source, ogg_int64_t offset, int whence);
  static int cb_close(void* source);
  static long cb_tell(void* source);

  PHYSFS_file*   file;
  OggVorbis_File vorbis_file;
  ogg_int64_t    loop_begin;
  ogg_int64_t    loop_at;
  size_t         normal_buffer_loop;
};

OggSoundFile::OggSoundFile(PHYSFS_file* file, double loop_begin, double loop_at)
{
  this->file = file;

  ov_callbacks callbacks = { cb_read, cb_seek, cb_close, cb_tell };
  ov_open_callbacks(file, &vorbis_file, 0, 0, callbacks);

  vorbis_info* vi = ov_info(&vorbis_file, -1);

  channels        = vi->channels;
  rate            = vi->rate;
  bits_per_sample = 16;
  size            = static_cast<size_t> (ov_pcm_total(&vorbis_file, -1) * 2);

  double samples_begin = loop_begin * rate;
  double sample_loop   = loop_at * rate;

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
  PHYSFS_file* file = reinterpret_cast<PHYSFS_file*> (source);

  PHYSFS_sint64 res
    = PHYSFS_read(file, ptr, static_cast<PHYSFS_uint32> (size),
        static_cast<PHYSFS_uint32> (nmemb));
  if(res <= 0)
    return 0;

  return static_cast<size_t> (res);
}

int
OggSoundFile::cb_seek(void* source, ogg_int64_t offset, int whence)
{
  PHYSFS_file* file = reinterpret_cast<PHYSFS_file*> (source);

  switch(whence) {
    case SEEK_SET:
      if(PHYSFS_seek(file, static_cast<PHYSFS_uint64> (offset)) == 0)
        return -1;
      break;
    case SEEK_CUR:
      if(PHYSFS_seek(file, PHYSFS_tell(file) + offset) == 0)
        return -1;
      break;
    case SEEK_END:
      if(PHYSFS_seek(file, PHYSFS_fileLength(file) + offset) == 0)
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
  PHYSFS_file* file = reinterpret_cast<PHYSFS_file*> (source);
  PHYSFS_close(file);
  return 0;
}

long
OggSoundFile::cb_tell(void* source)
{
  PHYSFS_file* file = reinterpret_cast<PHYSFS_file*> (source);
  return static_cast<long> (PHYSFS_tell(file));
}

//---------------------------------------------------------------------------

SoundFile* load_music_file(const std::string& filename)
{
  lisp::Parser parser(false);
  const lisp::Lisp* root = parser.parse(filename);
  const lisp::Lisp* music = root->get_lisp("supertux-music");
  if(music == NULL)
    throw SoundError("file is not a supertux-music file.");

  std::string raw_music_file;
  float loop_begin = 0;
  float loop_at    = -1;

  music->get("file", raw_music_file);
  music->get("loop-begin", loop_begin);
  music->get("loop-at", loop_at);
  
  if(loop_begin < 0) {
    throw SoundError("can't loop from negative value");
  }

  std::string basedir = FileSystem::dirname(filename);
  raw_music_file = FileSystem::normalize(basedir + raw_music_file);

  PHYSFS_file* file = PHYSFS_openRead(raw_music_file.c_str());
  if(!file) {
    std::stringstream msg;
    msg << "Couldn't open '" << raw_music_file << "': " << PHYSFS_getLastError();
    throw SoundError(msg.str());
  }

  return new OggSoundFile(file, loop_begin, loop_at);
}

SoundFile* load_sound_file(const std::string& filename)
{
  if(filename.length() > 6
      && filename.compare(filename.length()-6, 6, ".music") == 0) {
    return load_music_file(filename);
  }

  PHYSFS_file* file = PHYSFS_openRead(filename.c_str());
  if(!file) {
    std::stringstream msg;
    msg << "Couldn't open '" << filename << "': " << PHYSFS_getLastError() << ", using dummy sound file.";
    throw SoundError(msg.str());
  }

  try {
    char magic[4];
    if(PHYSFS_read(file, magic, sizeof(magic), 1) != 1)
      throw SoundError("Couldn't read magic, file too short");
    PHYSFS_seek(file, 0);
    if(strncmp(magic, "RIFF", 4) == 0)
      return new WavSoundFile(file);
    else if(strncmp(magic, "OggS", 4) == 0)
      return new OggSoundFile(file, 0, -1);
    else
      throw SoundError("Unknown file format");
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Couldn't read '" << filename << "': " << e.what();
    throw SoundError(msg.str());
  }
}
