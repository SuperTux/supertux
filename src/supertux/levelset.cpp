//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/levelset.hpp"

#include <physfs.h>
#include <algorithm>

#include "physfs/util.hpp"
#include "supertux/level_parser.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/string_util.hpp"

Levelset::Levelset(const std::string& basedir, bool recursively, bool parse_level_names) :
  m_basedir(basedir),
  m_levels(),
  m_level_names(),
  m_parse_level_names(parse_level_names)
{
  walk_directory(m_basedir, recursively);
  std::sort(m_levels.begin(), m_levels.end(), StringUtil::numeric_less);
  parse_names();
}

int
Levelset::get_num_levels() const
{
  return static_cast<int>(m_levels.size());
}

const std::string&
Levelset::get_level_filename(int i) const
{
  return m_levels[i];
}

const std::string*
Levelset::get_level_name(int i) const
{
  if (!m_parse_level_names)
    return nullptr;

  return &(m_level_names[i]);
}

void
Levelset::walk_directory(const std::string& directory, bool recursively)
{
  bool is_basedir = (directory == m_basedir);
  bool enumerateSuccess = physfsutil::enumerate_files_alphabetical(directory, [directory, is_basedir, recursively, this](const auto& filename) {
    auto filepath = FileSystem::join(directory, filename);
    if (physfsutil::is_directory(filepath) && recursively)
    {
      walk_directory(filepath, true);
    }
    if (StringUtil::has_suffix(filename, ".stl"))
    {
      if (is_basedir)
      {
        m_levels.push_back(filename);
      }
      else
      {
        // Replace basedir part of file path plus slash.
        filepath = filepath.replace(0, m_basedir.length() + 1, "");
        m_levels.push_back(filepath);
      }
    }
    return false;
  });

  if (!enumerateSuccess)
  {
    log_warning << "Couldn't read subset dir '" << directory << "'" << std::endl;
  }
}

void
Levelset::parse_names()
{
  if (!m_parse_level_names)
  {
    return;
  }

  m_level_names.reserve(m_levels.size());

  for(const auto& level_file : m_levels)
  {
    auto filepath = FileSystem::join(m_basedir, level_file);
    auto level_name = LevelParser::get_level_name(filepath);
    m_level_names.push_back(level_name);
  }
}
