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

#include "audio/sound_file.hpp"

#include <config.h>

#include <stdint.h>
#include <sstream>
#include <physfs.h>

#include "audio/sound_error.hpp"
#include "audio/ogg_sound_file.hpp"
#include "audio/wav_sound_file.hpp"
#include "lisp/parser.hpp"
#include "util/reader.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"

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

/* EOF */
