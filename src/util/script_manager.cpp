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
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/file_system.hpp"
#include "util/file_watcher.hpp"
#include "util/log.hpp"
#include "util/file_system.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <physfs.h>

static const char* TMP_DIR = "tmp";

ScriptManager::ScriptManager() :
  m_scripts(),
  m_watcher()
{
  PHYSFS_mkdir(TMP_DIR);

  clear_tmp();
}

ScriptManager::~ScriptManager()
{
  clear_tmp();
  m_scripts.clear();
}

void
ScriptManager::clear_tmp()
{
  m_watcher.clear();
  char** files = PHYSFS_enumerateFiles(TMP_DIR);

  for (char** f = files; *f; ++f)
  {
    log_debug << "Deleting leftover tmp file \"" << *f << "\"\n";
    PHYSFS_delete(FileSystem::join(TMP_DIR, *f).c_str());
  }

  PHYSFS_freeList(files);
}

std::string
ScriptManager::abspath_filename_from_key(UID uid, const std::string& key)
{
  // TODO: Expose something along Windows / $HOME/.cache on *nix
  return FileSystem::join(PHYSFS_getWriteDir(),
                          FileSystem::join(TMP_DIR, filename_from_key(uid, key)));
}

std::string
ScriptManager::relpath_filename_from_key(UID uid, const std::string& key)
{
  // TODO: Expose something along Windows / $HOME/.cache on *nix
  return FileSystem::join(TMP_DIR, filename_from_key(uid, key));
}

std::string
ScriptManager::filename_from_key(UID uid, const std::string& key)
{
  auto filename = "script_" + std::to_string(uid.get_value()) + '_' + key + ".nut";

#if defined(__APPLE__)
  // If no text editor is specified, use .txt extension to open in TextEdit,
  // otherwise MacOS has trouble finding the default text editor for .nut files.
  if(g_config->preferred_text_editor.empty())
  {
    return filename + ".txt";
  }
#endif

  return filename;
}

time_t
ScriptManager::get_mtime(UID uid, const std::string& key)
{
  return m_watcher.get_mtime(abspath_filename_from_key(uid, key));
}

auto
ScriptManager::find_script(UID uid, const std::string& key) -> decltype(m_scripts)::iterator
{
  auto res = std::find_if(m_scripts.begin(), m_scripts.end(),
    [&](const ScriptInfo& info) {
      return info.key == key && info.uid == uid;
    });

  return res;
}

bool
ScriptManager::is_script_registered(UID uid, const std::string& key)
{
  return find_script(uid, key) != m_scripts.end();
}

void
ScriptManager::register_script(UID uid, const std::string& key, std::string* script)
{
  std::string full_filename = abspath_filename_from_key(uid, key);

  log_debug << "Registering script \"" << filename_from_key(uid, key) << "\"" << std::endl;
  log_debug << "Full path \"" << full_filename << "\"" << std::endl;

  // TODO: Check if it's different. Causes some editors to nag.
  // Write current script contents
  std::ofstream file;
  file.open(full_filename);
  file << *script;
  file.close();

  if (is_script_registered(uid, key))
    return;
  log_debug << "Script wasn't registered, now monitoring.\n";
  m_scripts.push_back({uid, key, script});

  m_watcher.start_monitoring(full_filename,
    [this, uid, key](const FileWatcher::FileInfo& file) {
      log_debug << "Monitored script \"" << file.filename << "\" updated!\n";
      auto res = find_script(uid, key);
      if (res == m_scripts.end())
        return;
      log_debug << "This matched.\n";

      *res->script = "";
      std::fstream readme;
      readme.open(file.filename);
      std::string line;

      std::getline(readme, line);
      while (true)
      {
        *res->script += line;

        if (std::getline(readme, line))
          *res->script += "\n";
        else
          break;
      }
      readme.close();
  });
}

void
ScriptManager::poll()
{
  m_watcher.poll();
}
