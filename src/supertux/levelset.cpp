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

#include "util/log.hpp"
#include "util/string_util.hpp"

Levelset::Levelset(const std::string& basedir) :
  m_basedir(basedir),
  m_levels()
{
  char** files = PHYSFS_enumerateFiles(m_basedir.c_str());
  if (!files)
  {
    log_warning << "Couldn't read subset dir '" << m_basedir << "'" << std::endl;
    return;
  }

  for(const char* const* filename = files; *filename != 0; ++filename)
  {
    if(StringUtil::has_suffix(*filename, ".stl"))
    {
      m_levels.push_back(*filename);
    }
  }
  PHYSFS_freeList(files);

  std::sort(m_levels.begin(), m_levels.end(), StringUtil::numeric_less);
}

int
Levelset::get_num_levels() const
{
  return static_cast<int>(m_levels.size());
}

/* EOF */
