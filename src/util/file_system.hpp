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

#ifndef HEADER_SUPERTUX_UTIL_FILE_SYSTEM_HPP
#define HEADER_SUPERTUX_UTIL_FILE_SYSTEM_HPP

namespace FileSystem {

/** Returns true if the given path is a directory */
bool is_directory(const std::string& path);

/** Return true if the given file exists */
bool exists(const std::string& path);

/**
 *  Create the given directory
 */
void mkdir(const std::string& directory);

/**
 * returns the path of the directory the file is in
 */
std::string dirname(const std::string& filename);

/**
 * returns the name of the file
 */
std::string basename(const std::string& filename);

/**
 * remove everything starting from and including the last dot
 */
std::string strip_extension(const std::string& filename);

/**
 * normalize filename so that "blup/bla/blo/../../bar" will become
 * "blup/bar"
 */
std::string normalize(const std::string& filename);

/**
 * join two filenames join("foo", "bar") -> "foo/bar"
 */
std::string join(const std::string& lhs, const std::string& rhs);

} // namespace FileSystem

#endif

/* EOF */
