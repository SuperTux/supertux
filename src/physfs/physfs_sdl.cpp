//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "physfs/physfs_sdl.hpp"

#include <physfs.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_iostream.h>

#include "physfs/util.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <assert.h>
#include <stdio.h>

namespace {

Sint64 funcSize(void* userdata)
{
  PHYSFS_file* file = static_cast<PHYSFS_file*>(userdata);
  return (Sint64)PHYSFS_fileLength(file);
}

Sint64 funcSeek(void *userdata, Sint64 offset, SDL_IOWhence whence)
{
  PHYSFS_file* file = static_cast<PHYSFS_file*>(userdata);
  int res;
  switch (whence) {
    case SEEK_SET:
      res = PHYSFS_seek(file, offset);
      break;
    case SEEK_CUR:
      res = PHYSFS_seek(file, PHYSFS_tell(file) + offset);
      break;
    case SEEK_END:
      res = PHYSFS_seek(file, PHYSFS_fileLength(file) + offset);
      break;
    default:
      res = 0; // NOLINT
      assert(false);
      break;
  }
  if (res == 0) {
    log_warning << "Error seeking in file: " << physfsutil::get_last_error() << std::endl;
    return -1;
  }
  int i = static_cast<int>(PHYSFS_tell(file));


  return i;
}

size_t funcRead(void *userdata, void *ptr, size_t size, SDL_IOStatus *status)
{
  PHYSFS_file* file = static_cast<PHYSFS_file*>(userdata);

  PHYSFS_sint64 res = PHYSFS_readBytes(file, ptr, (PHYSFS_uint64)size);
  log_warning << "funcRead ERROR: " << physfsutil::get_last_error() << std::endl;
  if (res < 0)
  {
    return 0;
  }
  else
  {
    return static_cast<size_t>(res);
  }
}

size_t funcWrite(void *userdata, const void *ptr, size_t size, SDL_IOStatus *status)
{
  PHYSFS_file* file = static_cast<PHYSFS_file*>(userdata);

  PHYSFS_sint64 res = PHYSFS_writeBytes(file, ptr, size);
  if (res < 0)
  {
    return 0;
  }
  else
  {
    return static_cast<size_t>(res);
  }
}

bool funcFlush(void *userdata, SDL_IOStatus *status) {
    PHYSFS_file* file = static_cast<PHYSFS_file*>(userdata);

    if (PHYSFS_flush(file) != 0) {
        return true;
    }

    if (status != NULL) {
        return true;
    }

    return false;
}

bool funcClose(void *userdata)
{
  PHYSFS_file* file = static_cast<PHYSFS_file*>(userdata);

  PHYSFS_close(file);

  return false;
}

} // namespace

SDL_IOStream* get_physfs_SDLRWops(const std::string& filename)
{
  // check this as PHYSFS seems to be buggy and still returns a
  // valid pointer in this case
  if (filename.empty()) {
    throw std::runtime_error("Couldn't open file: empty filename");
  }

  PHYSFS_file* file = static_cast<PHYSFS_file*>(PHYSFS_openRead(filename.c_str()));
  if (!file) {
    std::stringstream msg;
    msg << "Couldn't open '" << filename << "': "
        << physfsutil::get_last_error();
    throw std::runtime_error(msg.str());
  }

  SDL_IOStreamInterface iface;
  SDL_INIT_INTERFACE(&iface);
  iface.size = funcSize;
  iface.seek = funcSeek;
  iface.read = funcRead;
  iface.write = funcWrite;
  iface.flush = funcFlush;
  iface.close = funcClose;
  return SDL_OpenIO(&iface, (void*)file);
}

SDL_IOStream* get_writable_physfs_SDLRWops(const std::string& filename)
{
  // check this as PHYSFS seems to be buggy and still returns a
  // valid pointer in this case
  if (filename.empty()) {
    throw std::runtime_error("Couldn't open file: empty filename");
  }

  PHYSFS_file* file = static_cast<PHYSFS_file*>(PHYSFS_openWrite(filename.c_str()));
  if (!file) {
    std::stringstream msg;
    msg << "Couldn't open '" << filename << "' for writing: "
        << physfsutil::get_last_error();
    throw std::runtime_error(msg.str());
  }

  SDL_IOStreamInterface iface;
  SDL_INIT_INTERFACE(&iface);
  iface.size = funcSize;
  iface.seek = funcSeek;
  iface.read = funcRead;
  iface.write = funcWrite;
  iface.flush = funcFlush;
  iface.close = funcClose;
  return SDL_OpenIO(&iface, (void*)file);
}
