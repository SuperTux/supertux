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

#pragma once

#include <memory>
#include <string>
#include <vector>

struct PHYSFS_File;

class SoundFile
{
public:
  enum FileFormat {
    FORMAT_WAV,
    FORMAT_OGG
  };

public:
  static FileFormat get_file_format(PHYSFS_File* file, const std::string& filename);

public:
  SoundFile() :
    m_channels(),
    m_rate(),
    m_bits_per_sample(),
    m_size(),
    m_authors(),
    m_license(),
    m_title()
  {}

  virtual ~SoundFile() {}

  virtual size_t read(void* buffer, size_t buffer_size) = 0;
  virtual void reset() = 0;

public:
  int m_channels;
  int m_rate;
  int m_bits_per_sample;
  /// size in bytes
  size_t m_size;
  std::vector<std::string> m_authors;
  std::string m_license;
  std::string m_title;

private:
  SoundFile(const SoundFile&) = delete;
  SoundFile& operator=(const SoundFile&) = delete;
};

std::unique_ptr<SoundFile> load_sound_file(const std::string& filename);
