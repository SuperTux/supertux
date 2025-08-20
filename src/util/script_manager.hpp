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
#include <functional>
#include <string>
#include <unordered_map>

class ScriptManager
{
public:
  using callback_t = std::function<void()>;
  
  struct ScriptInfo {
    std::string key;
    std::string* script;
    
    bool operator==(struct ScriptInfo& other) const {
      return other.key == key;
    }
    bool operator==(const std::string& other) const {
      return other == key;
    }
  };
public:
  ScriptManager();
  ~ScriptManager() = default;
  
  static std::string filename_from_key(const std::string& key);
  static std::string full_filename_from_key(const std::string& key);
  
  time_t get_mtime(const std::string& key);
  
  bool is_script_registered(const std::string& key);
  void register_script(std::string key, std::string* script);
  
  void poll();
private:
  
  std::vector<ScriptInfo> m_scripts;
  FileWatcher m_watcher;
};
