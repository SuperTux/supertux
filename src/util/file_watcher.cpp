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

#include <sys/stat.h>
#include "util/file_watcher.hpp"

FileWatcher::FileWatcher()
{
}

time_t
FileWatcher::get_mtime(const std::string &filename)
{
  struct stat file_stat;
  if (stat(filename.c_str(), &file_stat) != 0)
  {
    // TODO
    return 0;
  }

  return file_stat.st_mtime;
}

void
FileWatcher::start_monitoring(std::string filename, FileWatcher::callback_t fun)
{
  m_files.emplace(std::move(filename), FileWatcher::FileInfo{filename, get_mtime(filename), fun});
}

void
FileWatcher::poll()
{
  struct stat file_stat;
  for (auto &file : m_files)
  {
    FileInfo &finfo = file.second;
    time_t mtime = get_mtime(file.first);

    // Same time, don't bother
    if (finfo.m_last_mtime == mtime)
      continue;

    finfo.callback(finfo);
    finfo.m_last_mtime = mtime;
  }
}

void
FileWatcher::clear()
{
  m_files.clear();
}
