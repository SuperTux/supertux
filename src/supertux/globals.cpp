//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/globals.hpp"

#include <tinygettext/tinygettext.hpp>

#include "supertux/gameconfig.hpp"

int SCREEN_WIDTH;
int SCREEN_HEIGHT;

std::unique_ptr<Config> g_config;

float game_time = 0;
float real_time = 0;

TileSet* current_tileset = NULL;

float g_game_speed = 1.0f;

/* EOF */
