//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <assert.h>

#include "tilemap.h"
#include "display_manager.h"
#include "level.h"
#include "tile.h"
#include "globals.h"

TileMap::TileMap(DisplayManager& display_manager, Level* newlevel)
  : level(newlevel)
{
  display_manager.add_drawable(this, LAYER_BACKGROUNDTILES);
  display_manager.add_drawable(this, LAYER_TILES);
  display_manager.add_drawable(this, LAYER_FOREGROUNDTILES);
}

TileMap::~TileMap()
{
}

void
TileMap::action(float )
{
}

void
TileMap::draw(ViewPort& viewport, int layer)
{
  std::vector<unsigned int>* tiles;
  switch(layer) {
    case LAYER_BACKGROUNDTILES:
      tiles = &level->bg_tiles; break;
    case LAYER_TILES:
      tiles = &level->ia_tiles; break;
    case LAYER_FOREGROUNDTILES:
      tiles = &level->fg_tiles; break;
    default:
      assert(!"Wrong layer when drawing tilemap.");
  }

  int tsx = int(viewport.get_translation().x / 32); // tilestartindex x
  int tsy = int(viewport.get_translation().y / 32); // tilestartindex y
  int sx = - (int(viewport.get_translation().x) % 32);
  int sy = - (int(viewport.get_translation().y) % 32);
  for(int x = sx, tx = tsx; x < screen->w && tx < level->width;
      x += 32, ++tx) {
    for(int y = sy, ty = tsy; y < screen->h && ty < level->height;
          y += 32, ++ty) {
      Tile::draw(x, y, (*tiles) [ty * level->width + tx]);
    }
  }
}
