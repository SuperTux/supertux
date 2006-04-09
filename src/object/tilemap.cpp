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
#include <config.h>

#include <cassert>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <cmath>

#include "tilemap.hpp"
#include "video/drawing_context.hpp"
#include "level.hpp"
#include "tile.hpp"
#include "resources.hpp"
#include "tile_manager.hpp"
#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"
#include "object_factory.hpp"
#include "main.hpp"
#include "msg.hpp"

TileMap::TileMap()
  : solid(false), speed(1), width(0), height(0), layer(LAYER_TILES),
    drawing_effect(NO_EFFECT)
{
  tilemanager = tile_manager;

  if(solid)
    flags |= FLAG_SOLID;
}

TileMap::TileMap(const lisp::Lisp& reader, TileManager* new_tile_manager)
  : solid(false), speed(1), width(-1), height(-1), layer(LAYER_TILES),
    drawing_effect(NO_EFFECT)
{
  tilemanager = new_tile_manager;
  if(tilemanager == 0)
    tilemanager = tile_manager;

  std::string layer_str;
  if(reader.get("layer", layer_str)) {
    if(layer_str == "background")
      layer = LAYER_BACKGROUNDTILES;
    else if(layer_str == "interactive")
      layer = LAYER_TILES;
    else if(layer_str == "foreground")
      layer = LAYER_FOREGROUNDTILES;
    else
      msg_warning << "Unknown layer '" << layer_str << "' in tilemap" << std::endl;
  }

  reader.get("solid", solid);
  reader.get("speed", speed);

  if(solid && speed != 1) {
    msg_warning << "Speed of solid tilemap is not 1. fixing" << std::endl;
    speed = 1;
  }
  if(solid)
    flags |= FLAG_SOLID;
 
  reader.get("width", width);
  reader.get("height", height);
  if(width < 0 || height < 0)
    throw std::runtime_error("Invalid/No width/height specified in tilemap.");

  if(!reader.get_vector("tiles", tiles))
    throw std::runtime_error("No tiles in tilemap.");

  if(int(tiles.size()) != width*height) {
    throw std::runtime_error("wrong number of tiles in tilemap.");
  }

  // make sure all tiles are loaded
  for(Tiles::iterator i = tiles.begin(); i != tiles.end(); ++i)
    tilemanager->get(*i);
}

TileMap::TileMap(int layer_, bool solid_, size_t width_, size_t height_)
  : solid(solid_), speed(1), width(0), height(0), layer(layer_),
    drawing_effect(NO_EFFECT)
{
  tilemanager = tile_manager;
  
  resize(width_, height_);

  if(solid)
    flags |= FLAG_SOLID;  
}

TileMap::~TileMap()
{
}

void
TileMap::write(lisp::Writer& writer)
{
  writer.start_list("tilemap");

  if(layer == LAYER_BACKGROUNDTILES)
    writer.write_string("layer", "background");
  else if(layer == LAYER_TILES)
    writer.write_string("layer", "interactive");
  else if(layer == LAYER_FOREGROUNDTILES)
    writer.write_string("layer", "foreground");
  else {
    writer.write_string("layer", "unknown");
    msg_warning << "unknown layer in tilemap" << std::endl;
  }

  writer.write_bool("solid", solid);
  writer.write_float("speed", speed);
  writer.write_int("width", width);
  writer.write_int("height", height);
  writer.write_int_vector("tiles", tiles);
  
  writer.end_list("tilemap");
}

void
TileMap::update(float )
{
}

void
TileMap::draw(DrawingContext& context)
{
  context.push_transform();

  if(drawing_effect != 0)
    context.set_drawing_effect(drawing_effect); 
  float trans_x = roundf(context.get_translation().x);
  float trans_y = roundf(context.get_translation().y);
  context.set_translation(Vector(trans_x * speed, trans_y * speed));

  /** if we don't round here, we'll have a 1 pixel gap on screen sometimes.
   * I have no idea why */
  float start_x = roundf(context.get_translation().x);
  if(start_x < 0)
    start_x = 0;
  float start_y = roundf(context.get_translation().y);
  if(start_y < 0)
    start_y = 0;
  float end_x = std::min(start_x + SCREEN_WIDTH, float(width * 32));
  float end_y = std::min(start_y + SCREEN_HEIGHT, float(height * 32));
  start_x -= int(start_x) % 32;
  start_y -= int(start_y) % 32;  
  int tsx = int(start_x / 32); // tilestartindex x
  int tsy = int(start_y / 32); // tilestartindex y

  Vector pos;
  int tx, ty;
  for(pos.x = start_x, tx = tsx; pos.x < end_x; pos.x += 32, ++tx) {
    for(pos.y = start_y, ty = tsy; pos.y < end_y; pos.y += 32, ++ty) {
      const Tile* tile = tilemanager->get(tiles[ty*width + tx]);
      assert(tile != 0);
      tile->draw(context, pos, layer);
    }
  }

#if 0
  if (debug_grid)
  {
    for (pos.x = start_x; pos.x < end_x; pos.x += 32)
    {
       context.draw_filled_rect(Vector (pos.x, start_y), Vector(1, fabsf(start_y - end_y)),
                  Color(0.8f, 0.8f, 0.8f), LAYER_GUI-50);
    }

    for (pos.y = start_y; pos.y < end_y; pos.y += 32)
    {
       context.draw_filled_rect(Vector (start_x, pos.y), Vector(fabsf(start_x - end_x), 1),
                  Color(1.0f, 1.0f, 1.0f), LAYER_GUI-50);
    }
  }
#endif

  context.pop_transform();
}

void
TileMap::set(int newwidth, int newheight, const std::vector<unsigned int>&newt,
    int newlayer, bool newsolid)
{
  if(int(newt.size()) != newwidth * newheight)
    throw std::runtime_error("Wrong tilecount count.");

  width  = newwidth;
  height = newheight;

  tiles.resize(newt.size());
  tiles = newt;

  layer  = newlayer;
  solid  = newsolid;
  if(solid)
    flags |= FLAG_SOLID;

  // make sure all tiles are loaded
  for(Tiles::iterator i = tiles.begin(); i != tiles.end(); ++i)
    tilemanager->get(*i);                                        
}

void
TileMap::resize(int new_width, int new_height)
{
  if(new_width < width) {
    // remap tiles for new width
    for(int y = 0; y < height && y < new_height; ++y) {
      for(int x = 0; x < new_width; ++x) {
        tiles[y * new_width + x] = tiles[y * width + x];
      }
    }
  }
                                                                                
  tiles.resize(new_width * new_height);
                                                                                
  if(new_width > width) {
    // remap tiles
    for(int y = std::min(height, new_height)-1; y >= 0; --y) {
      for(int x = new_width-1; x >= 0; --x) {
        if(x >= width) {
          tiles[y * new_width + x] = 0;
          continue;
        }
        
        tiles[y * new_width + x] = tiles[y * width + x];
      }
    }
  }

  height = new_height;
  width = new_width;
}

const Tile*
TileMap::get_tile(int x, int y) const
{
  if(x < 0 || x >= width || y < 0 || y >= height) {
    //msg_warning << "tile outside tilemap requested" << std::endl;
    return tilemanager->get(0);
  }

  return tilemanager->get(tiles[y*width + x]);
}

const Tile*
TileMap::get_tile_at(const Vector& pos) const
{
  return get_tile(int(pos.x)/32, int(pos.y)/32);
}

void
TileMap::change(int x, int y, uint32_t newtile)
{
  assert(x >= 0 && x < width && y >= 0 && y < height);
  tiles[y*width + x] = newtile;
}

void
TileMap::change_at(const Vector& pos, uint32_t newtile)
{
  change(int(pos.x)/32, int(pos.y)/32, newtile);
}

void
TileMap::change_all(uint32_t oldtile, uint32_t newtile)
{
  for (size_t x = 0; x < get_width(); x++)
    for (size_t y = 0; y < get_height(); y++) {
      if (get_tile(x,y)->getID() == oldtile) change(x,y,newtile);
    }
}

IMPLEMENT_FACTORY(TileMap, "tilemap");
