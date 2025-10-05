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

SDL_IOStream* get_physfs_SDLRWops(const std::string& filename)
{
  // check this as PHYSFS seems to be buggy and still returns a
  // valid pointer in this case
  if (filename.empty()) {
    throw std::runtime_error("Couldn't open file: empty filename");
  }

  auto path = PHYSFS_getRealDir(filename.c_str());
  if (!path) {
    std::stringstream msg;
    msg << "File '" << filename << "' doesn't exist in any search path";
    throw std::runtime_error(msg.str());
  }

  auto full_path = FileSystem::join(path, filename);
  return SDL_IOFromFile(full_path.c_str(), "rb");
}

SDL_IOStream* get_writable_physfs_SDLRWops(const std::string& filename)
{
  // check this as PHYSFS seems to be buggy and still returns a
  // valid pointer in this case
  if (filename.empty()) {
    throw std::runtime_error("Couldn't open file: empty filename");
  }

  auto path = PHYSFS_getRealDir(filename.c_str());
  if (!path) {
    std::stringstream msg;
    msg << "File '" << filename << "' doesn't exist in any search path";
    throw std::runtime_error(msg.str());
  }

  auto full_path = FileSystem::join(path, filename);
  return SDL_IOFromFile(full_path.c_str(), "wb");
}
