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
    UID key;
    std::string* script;
    
    bool operator==(struct ScriptInfo& other) const {
      return other.key == key;
    }
    bool operator==(const UID& other) const {
      return other == key;
    }
  };
public:
  ScriptManager();
  ~ScriptManager() = default;
  
  static std::string filename_from_key(UID key);
  static std::string full_filename_from_key(UID key);
  
  time_t get_mtime(UID key);
  
  bool is_script_registered(UID key);
  void register_script(UID key, std::string* script);
  
  void poll();
private:
  
  std::vector<ScriptInfo> m_scripts;
  FileWatcher m_watcher;
};
