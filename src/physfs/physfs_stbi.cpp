//  SuperTux
//  Copyright (C) 2023 mrkubax10 <mrkubax10@onet.pl>
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

#include "physfs/physfs_stbi.hpp"

#include <physfs.h>

static int
callback_read(void* user, char* data, int size)
{
  PHYSFS_File* file = reinterpret_cast<PHYSFS_File*>(user);
  return static_cast<int>(PHYSFS_readBytes(file, data, size));
}

static void
callback_skip(void* user, int n)
{
  PHYSFS_File* file = reinterpret_cast<PHYSFS_File*>(user);
  PHYSFS_seek(file, PHYSFS_tell(file)+n);
}

static int
callback_eof(void* user)
{
  PHYSFS_File* file = reinterpret_cast<PHYSFS_File*>(user);
  return PHYSFS_eof(file);
}

std::unique_ptr<stbi_io_callbacks>
get_physfs_stbi_io_callbacks(const std::string& filename, PHYSFS_File*& file)
{
  std::unique_ptr<stbi_io_callbacks> result;
  file = PHYSFS_openRead(filename.c_str());
  if(!file)
    return result;

  result.reset(new stbi_io_callbacks);
  result->read = callback_read;
  result->skip = callback_skip;
  result->eof = callback_eof;
  return result;
}

void
physfs_stbi_write_func(void* context, void* data, int size)
{
  PHYSFS_File* file = reinterpret_cast<PHYSFS_File*>(context);
  PHYSFS_writeBytes(file, data, size);
}

/* EOF */
