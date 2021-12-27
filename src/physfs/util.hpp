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

#ifndef HEADER_SUPERTUX_PHYSFS_UTIL_HPP
#define HEADER_SUPERTUX_PHYSFS_UTIL_HPP

#include <string>

namespace physfsutil {

/** Convert 'path' to it's canonical name, i.e. normalize it and add a
    '/' to the front) */
std::string realpath(const std::string& path);

/** Returns true if the given path is a directory or a symlink
    pointing to a directory */
bool is_directory(const std::string& path);

bool remove(const std::string& filenam);

/** Removes directory with content */
void remove_with_content(const std::string& filename);

} // namespace physfsutil

#endif

/* EOF */
