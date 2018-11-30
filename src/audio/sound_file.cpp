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

#include <string.h>
#include <physfs.h>
#include <sstream>

#include "audio/ogg_sound_file.hpp"
#include "audio/sound_error.hpp"
#include "audio/wav_sound_file.hpp"
#include "util/file_system.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/string_util.hpp"

namespace {

std::unique_ptr<SoundFile> load_music_file(const std::string& filename)
{
  auto doc = ReaderDocument::from_file(filename);
  auto root = doc.get_root();
  if (root.get_name() != "supertux-music")
  {
    throw SoundError("file is not a supertux-music file.");
  }
  else
  {
    auto music = root.get_mapping();

    std::string raw_music_file;
    float loop_begin = 0;
    float loop_at    = -1;

    music.get("file", raw_music_file);
    music.get("loop-begin", loop_begin);
    music.get("loop-at", loop_at);

    if (loop_begin < 0) {
      throw SoundError("can't loop from negative value");
    }

    std::string basedir = FileSystem::dirname(filename);
    raw_music_file = FileSystem::normalize(basedir + raw_music_file);

    auto file = PHYSFS_openRead(raw_music_file.c_str());
    if (!file) {
      std::stringstream msg;
      msg << "Couldn't open '" << raw_music_file << "': " << PHYSFS_getLastErrorCode();
      throw SoundError(msg.str());
    }
    auto format = SoundFile::get_file_format(file, raw_music_file);
    if (format == SoundFile::FORMAT_WAV)
    {
      return std::make_unique<WavSoundFile>(file);
    }
    else
    {
      return std::make_unique<OggSoundFile>(file, loop_begin, loop_at);
    }
  }
}

} // namespace

std::unique_ptr<SoundFile> load_sound_file(const std::string& filename)
{
  if (StringUtil::has_suffix(filename, ".music")) {
    return load_music_file(filename);
  }

  auto file = PHYSFS_openRead(filename.c_str());
  if (!file) {
    std::stringstream msg;
    msg << "Couldn't open '" << filename << "': " << PHYSFS_getLastErrorCode() << ", using dummy sound file.";
    throw SoundError(msg.str());
  }

  auto format = SoundFile::get_file_format(file, filename);
  if (format == SoundFile::FORMAT_WAV)
  {
    return std::make_unique<WavSoundFile>(file);
  }
  else
  {
    return std::make_unique<OggSoundFile>(file, 0, -1);
  }
}

SoundFile::FileFormat
SoundFile::get_file_format(PHYSFS_File* file, const std::string& filename)
{
  try {
    char magic[4];
    if (PHYSFS_readBytes(file, magic, sizeof(magic)) < static_cast<std::make_signed<size_t>::type>(sizeof(magic)))
      throw SoundError("Couldn't read magic, file too short");
    if (PHYSFS_seek(file, 0) == 0) {
      std::stringstream msg;
      msg << "Couldn't seek through sound file: " << PHYSFS_getLastErrorCode();
      throw SoundError(msg.str());
    }

    if (strncmp(magic, "RIFF", 4) == 0)
      return FileFormat::FORMAT_WAV;
    else if (strncmp(magic, "OggS", 4) == 0)
      return FileFormat::FORMAT_OGG;
    else
      throw SoundError("Unknown file format");
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Couldn't read '" << filename << "': " << e.what();
    throw SoundError(msg.str());
  }
}

/* EOF */
