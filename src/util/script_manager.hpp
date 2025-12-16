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

#pragma once

#include "util/file_watcher.hpp"
#include "util/uid.hpp"
#include <functional>
#include <string>
#include <vector>

class ScriptManager
{
public:
  using callback_t = std::function<void()>;

  struct ScriptInfo {
    UID uid;
    std::string key;
    std::string* script;

    bool operator==(const struct ScriptInfo& other) const {
      return other.uid == uid && other.key == key;
    }
    bool operator==(const UID& other) const {
      return other == uid;
    }
    bool operator==(const std::string& other) const {
      return other == key;
    }
  };

public:
  ScriptManager();
  ~ScriptManager();

  static std::string filename_from_key(UID uid, const std::string& key);
  static std::string abspath_filename_from_key(UID uid, const std::string& key);
  static std::string relpath_filename_from_key(UID uid, const std::string& key);

  time_t get_mtime(UID uid, const std::string& key);

  bool is_script_registered(UID uid, const std::string& key);
  void register_script(UID uid, const std::string& key, std::string* script);

  void poll();

  void clear_tmp();

private:
  std::vector<ScriptInfo> m_scripts;
  decltype(m_scripts)::iterator find_script(UID uid, const std::string& key);

  FileWatcher m_watcher;
};
