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

#pragma once

#include <string>

namespace FileSystem {

/** Returns true if the given path is a directory */
bool is_directory(const std::string& path);

/** Return true if the given file exists */
bool exists(const std::string& path);

/** Create the given directory */
void mkdir(const std::string& directory);

/** Copy file from one directory to another */
void copy(const std::string& source_path, const std::string& target_path);

/** returns the path of the directory the file is in */
std::string dirname(const std::string& filename);

/** returns the name of the file 
 * @param filename The path to get the basename from.
 * @param greedy If true, then attempt to strip any slashes from the end first.
 *               This fixes situations like /some/dir/ where they really meant /some/dir.
 */
std::string basename(std::string path, bool greedy = false);

/** Return a path to 'filename' that is relative to 'basedir', e.g.
    reldir("/levels/juser/level1.stl", "/levels") -> "juser/level1.stl" */
std::string relpath(const std::string& filename, const std::string& basedir);

/** Return the extension of a file */
std::string extension(const std::string& filename);

/** remove everything starting from and including the last dot */
std::string strip_extension(const std::string& filename);

/** strip any leading paths, like /some/path///// */
std::string strip_leading_dirs(std::string filename);

/** normalize filename so that "blup/bla/blo/../../bar" will become
    "blup/bar" */
std::string normalize(const std::string& filename);

/** join two filenames join("foo", "bar") -> "foo/bar" */
std::string join(const std::string& lhs, const std::string& rhs);

/** Escapes path components */
std::string escape_url(const std::string& url);

/** Remove a file
    @return true when successfully removed, false otherwise */
 bool remove(const std::string& path);

/** Opens a file path with the user's preferred app for that file.
 * @param path path to open
 */
 void open_path(const std::string& path);
 
/** Opens a file in the users preferred text editor.
 * @param filename File to edit
 */
void open_editor(const std::string& filename);

/** Opens an URL in the user's preferred browser.
 * @param url URL to open
 */
 void open_url(const std::string& url);

} // namespace FileSystem
