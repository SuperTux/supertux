//  SuperTux
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

#ifdef __vita__

#include <string>

#include <physfs.h>

PHYSFS_Io* vita_open_archive_io(const char* path);
PHYSFS_sint64 vita_archive_offset(const std::string& filename);
void vita_archive_prefetch(const std::string& filename);

#endif
