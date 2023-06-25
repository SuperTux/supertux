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

#ifndef HEADER_SUPERTUX_PHYSFS_PHYSFS_STBI_HPP
#define HEADER_SUPERTUX_PHYSFS_PHYSFS_STBI_HPP

#include <memory>
#include <string>

#include <stb_image.h>

struct PHYSFS_File;

std::unique_ptr<stbi_io_callbacks> get_physfs_stbi_io_callbacks(const std::string& filename, PHYSFS_File*& file);
void physfs_stbi_write_func(void* context, void* data, int size);

#endif

/* EOF */
