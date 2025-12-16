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

#include <ctime>
#include <functional>
#include <string>
#include <unordered_map>

class FileWatcher
{
public:
  struct FileInfo;
  using callback_t = std::function<void(FileInfo&)>;

  struct FileInfo {
  	std::string filename;
    time_t m_last_mtime;
    callback_t callback;

    bool operator==(struct FileInfo& other) const {
      return other.filename == filename;
    }
    bool operator==(const std::string& other) const {
      return other == filename;
    }
  };
public:
  FileWatcher();
  ~FileWatcher() = default;

  void start_monitoring(std::string filename, callback_t fun);

  void poll();
  time_t get_mtime(const std::string &filename);

  void clear();
private:

  std::unordered_map<std::string, FileInfo> m_files;
};
