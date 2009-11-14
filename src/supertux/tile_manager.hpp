//  SuperTux
//  Copyright (C) 2008 Matthias Braun <matze@braunis.de>
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
#ifndef HEADER_TILE_MANAGER_HPP
#define HEADER_TILE_MANAGER_HPP

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <stdint.h>
#include <assert.h>
#include "util/log.hpp"

namespace lisp {
class Lisp;
}

class TileSet;

class TileManager
{
private:
  typedef std::map<std::string, TileSet*> TileSets;
  TileSets tilesets;

public:
  TileManager();
  ~TileManager();

  TileSet* get_tileset(const std::string &filename);

  TileSet* parse_tileset_definition(const lisp::Lisp& reader);
};

extern TileManager *tile_manager;
/** this is only set while loading a map */
extern TileSet     *current_tileset;

#endif
