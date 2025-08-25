//  SuperTux
//  Copyright (C) 2025 Hyland B. <me@ow.swag.toys>
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

#include "script_manager.hpp"
#include "util/file_system.hpp"
#include "util/file_watcher.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <physfs.h>

ScriptManager::ScriptManager() :
  m_scripts(),
  m_watcher()
{

}

bool
ScriptManager::is_script_registered(UID key)
{
  auto res = std::find(m_scripts.begin(), m_scripts.end(), key);
  return res != m_scripts.end();
}

std::string
ScriptManager::full_filename_from_key(UID key)
{
  return FileSystem::join(PHYSFS_getWriteDir(), "tmp/" + filename_from_key(key));
}

std::string
ScriptManager::filename_from_key(UID key)
{
  return "script_" + std::to_string(key.get_value()) + ".nut";
}

time_t
ScriptManager::get_mtime(UID key)
{
  return m_watcher.get_mtime(full_filename_from_key(key));
}

void
ScriptManager::register_script(UID key, std::string* script)
{
  std::string full_filename = full_filename_from_key(key);

  // TODO: Check if it's different. Causes some editors to nag.  
  // Write current script contents
  std::ofstream file;
  file.open(full_filename);
  file << *script;
  file.close();
  
  if (is_script_registered(key))
    return;
  m_scripts.push_back({key, script});

  m_watcher.start_monitoring(full_filename, [this, key](FileWatcher::FileInfo& file) {
      auto res = std::find(m_scripts.begin(), m_scripts.end(), key);
      if (res == m_scripts.end())
        return;
      
      *res->script = "";
      std::fstream readme;
      readme.open(file.filename);
      std::string line;
      while (std::getline(readme, line))
      {
        *res->script += line + "\n";
      }
      readme.close();
  });
}

void
ScriptManager::poll()
{
  m_watcher.poll();
}
