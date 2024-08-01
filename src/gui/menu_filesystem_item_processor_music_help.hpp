//  SuperTux
//  Copyright (C) 2024 James De Ricco <james.dericco.dev@gmail.com>
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

#ifndef HEADER_SUPERTUX_GUI_MENU_FILESYSTEM_ITEM_PROCESSOR_MUSIC_HELP_HPP
#define HEADER_SUPERTUX_GUI_MENU_FILESYSTEM_ITEM_PROCESSOR_MUSIC_HELP_HPP

#include "audio/sound_file.hpp"
#include "gui/menu_item.hpp"

void item_processor_music_help(MenuItem& menu_item, const std::string& file_path, bool in_basedir);

#endif

/* EOF */
