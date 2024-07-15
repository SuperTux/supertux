//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "physfs/util.hpp"

#include <physfs.h>

#include "physfs/physfs_file_system.hpp"
#include "util/file_system.hpp"

namespace physfsutil {

const char* get_last_error()
{
  return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
}

std::string realpath(const std::string& path)
{
  std::string result = FileSystem::normalize(path);
  if (result.empty()) {
    return "/";
  } else if (result[0] != '/') {
    return '/' + result;
  } else  {
    return result;
  }
}

bool is_directory(const std::string& path)
{
  PHYSFS_Stat statbuf;
  if (!PHYSFS_stat(path.c_str(), &statbuf)) {
    return false;
  } else {
    if (statbuf.filetype == PHYSFS_FILETYPE_SYMLINK) {
      // PHYSFS_stat() doesn't follow symlinks, so we do it manually
      const char* realdir = PHYSFS_getRealDir(path.c_str());
      if (realdir == nullptr) {
        return false;
      } else {
        const std::string realfname = FileSystem::join(realdir, path);
        return FileSystem::is_directory(realfname);
      }
    } else {
      return statbuf.filetype == PHYSFS_FILETYPE_DIRECTORY;
    }
  }
}

bool remove(const std::string& filename)
{
  return PHYSFS_delete(filename.c_str()) == 0;
}

#define PHYSFS_UTIL_DIRECTORY_GUARD \
  if (!is_directory(dir) || !PHYSFS_exists(dir.c_str())) return

void remove_content(const std::string& dir)
{
  PHYSFS_UTIL_DIRECTORY_GUARD;
  enumerate_files(dir, [&dir](const std::string& file) {
    std::string path = FileSystem::join(dir, file);
    if (is_directory(path))
      remove_with_content(path);
    PHYSFS_delete(path.c_str());
  });
}

void remove_with_content(const std::string& dir)
{
  PHYSFS_UTIL_DIRECTORY_GUARD;

  remove_content(dir);
  remove(dir);
}

bool enumerate_files(const std::string& pathname, std::function<void(const std::string&)> callback)
{
  std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
  files(PHYSFS_enumerateFiles(pathname.c_str()),
        PHYSFS_freeList);

  if(files == nullptr)
    return false;

  for (const char* const* filename = files.get(); *filename != nullptr; ++filename)
  {
    callback(*filename);
  }

  return true;
}

} // namespace physfsutil

/* EOF */
