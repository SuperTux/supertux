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
#include "tilemap.h"

#include <assert.h>
#include <algorithm>
#include <math.h>
#include "screen/drawing_context.h"
#include "level.h"
#include "tile.h"
#include "globals.h"

TileMap::TileMap(Level* newlevel)
  : level(newlevel)
{
  tilemanager = TileManager::instance();
}

TileMap::~TileMap()
{
}

void
TileMap::action(float )
{
}

void
TileMap::draw(const std::vector<unsigned int>& tiles, DrawingContext& context,
    int layer)
{
  /** if we don't round here, we'll have a 1 pixel gap on screen sometimes.
   * I have no idea why */
  float start_x = roundf(context.get_translation().x);
  float start_y = roundf(context.get_translation().y);
  float end_x = std::min(start_x + screen->w, float(level->width * 32));
  float end_y = std::min(start_y + screen->h, float(level->height * 32));
  start_x -= int(start_x) % 32;
  start_y -= int(start_y) % 32;  
  int tsx = int(start_x / 32); // tilestartindex x
  int tsy = int(start_y / 32); // tilestartindex y
  
  Vector pos;
  int tx, ty;
  for(pos.x = start_x, tx = tsx; pos.x < end_x; pos.x += 32, ++tx) {
    for(pos.y = start_y, ty = tsy; pos.y < end_y; pos.y += 32, ++ty) {
      tilemanager->draw_tile(context, tiles[ty*level->width + tx], pos, layer);
    }
  }
}

void
TileMap::draw(DrawingContext& context)
{
  draw(level->bg_tiles, context, LAYER_BACKGROUNDTILES);
  draw(level->ia_tiles, context, LAYER_TILES);
  draw(level->fg_tiles, context, LAYER_FOREGROUNDTILES);
}
