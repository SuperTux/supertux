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

//
// TEST DISABLED:
//  - We should eventually remove our own file_system stuff and just use std::filesystem
//  - Dependencies on SDL2.. OpenGL.. Gettext.. etc.. Wtf?
//

#include "st_assert.hpp"
#include "util/file_system.hpp"


int main(void)
{
  ST_ASSERT(std::nullopt, FileSystem::join("foo/bar", "") == "foo/bar/");
  ST_ASSERT(std::nullopt, FileSystem::join("", "foo/bar") == "foo/bar");
  ST_ASSERT(std::nullopt, FileSystem::join("foo/bar", "baz/boing") == "foo/bar/baz/boing");
  ST_ASSERT(std::nullopt, FileSystem::join("foo/bar", "/baz/boing") == "foo/bar/baz/boing");
  ST_ASSERT(std::nullopt, FileSystem::join("foo/bar/", "/baz/boing") == "foo/bar/baz/boing");
  ST_ASSERT(std::nullopt, FileSystem::join("/foo/bar", "baz/boing") == "/foo/bar/baz/boing");
  
  // Relpath tests
  ST_ASSERT(std::nullopt, FileSystem::relpath("/levels/juser/level.stl", "/") == "levels/juser/level.stl");
  ST_ASSERT(std::nullopt, FileSystem::relpath("/levels/juser/level.stl", "/levels") == "juser/level.stl");
  ST_ASSERT(std::nullopt, FileSystem::relpath("/levels/juser/level.stl", "/levels/juser") == "level.stl");
}

/* EOF */
