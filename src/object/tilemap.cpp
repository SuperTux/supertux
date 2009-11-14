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

#include <config.h>

#include <cassert>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <math.h>
#include <limits>

#include "tilemap.hpp"
#include "video/drawing_context.hpp"
#include "level.hpp"
#include "tile.hpp"
#include "resources.hpp"
#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"
#include "lisp/writer.hpp"
#include "object_factory.hpp"
#include "main.hpp"
#include "util/log.hpp"
#include "tile_set.hpp"
#include "tile_manager.hpp"
#include "scripting/tilemap.hpp"
#include "scripting/squirrel_util.hpp"

TileMap::TileMap(const TileSet *new_tileset)
  : tileset(new_tileset), solid(false), speed_x(1), speed_y(1), width(0),
    height(0), z_pos(0), x_offset(0), y_offset(0), movement(Vector(0,0)), drawing_effect(NO_EFFECT),
    alpha(1.0), current_alpha(1.0), remaining_fade_time(0),
    draw_target(DrawingContext::NORMAL)
{
}

TileMap::TileMap(const lisp::Lisp& reader)
  : solid(false), speed_x(1), speed_y(1), width(-1),
    height(-1), z_pos(0), x_offset(0), y_offset(0), movement(Vector(0,0)), drawing_effect(NO_EFFECT),
    alpha(1.0), current_alpha(1.0), remaining_fade_time(0),
    draw_target(DrawingContext::NORMAL)
{
  tileset = current_tileset;
  assert(tileset != NULL);

  reader.get("name",   name);
  reader.get("z-pos",  z_pos);
  reader.get("solid",  solid);
  reader.get("speed",  speed_x);
  reader.get("speed-y", speed_y);
  
  if(solid && ((speed_x != 1) || (speed_y != 1))) {
    log_warning << "Speed of solid tilemap is not 1. fixing" << std::endl;
    speed_x = 1;
    speed_y = 1;
  }

  const lisp::Lisp* pathLisp = reader.get_lisp("path");
  if (pathLisp) {
    path.reset(new Path());
    path->read(*pathLisp);
    walker.reset(new PathWalker(path.get(), /*running*/false));
    Vector v = path->get_base();
    set_x_offset(v.x);
    set_y_offset(v.y);
  }

  std::string draw_target_s = "normal";
  reader.get("draw-target", draw_target_s);
  if (draw_target_s == "normal") draw_target = DrawingContext::NORMAL;
  if (draw_target_s == "lightmap") draw_target = DrawingContext::LIGHTMAP;

  if (reader.get("alpha", alpha)) {
    current_alpha = alpha;
  }

  reader.get("width", width);
  reader.get("height", height);
  if(width < 0 || height < 0)
    throw std::runtime_error("Invalid/No width/height specified in tilemap.");

  if(!reader.get("tiles", tiles))
    throw std::runtime_error("No tiles in tilemap.");

  if(int(tiles.size()) != width*height) {
    throw std::runtime_error("wrong number of tiles in tilemap.");
  }

  bool empty = true;

  // make sure all tiles used on the tilemap are loaded and tilemap isn't empty
  for(Tiles::iterator i = tiles.begin(); i != tiles.end(); ++i) {
    if(*i != 0) {
      empty = false;
    }

    tileset->get(*i);
  }

  if(empty)
    log_info << "Tilemap '" << name << "', z-pos '" << z_pos << "' is empty." << std::endl;
}

TileMap::TileMap(const TileSet *new_tileset, std::string name, int z_pos,
                 bool solid, size_t width, size_t height)
  : tileset(new_tileset), solid(solid), speed_x(1), speed_y(1), width(0),
    height(0), z_pos(z_pos), x_offset(0), y_offset(0), movement(Vector(0,0)),
    drawing_effect(NO_EFFECT), alpha(1.0), current_alpha(1.0),
    remaining_fade_time(0), draw_target(DrawingContext::NORMAL)
{
  this->name = name;

  resize(width, height);
}

TileMap::~TileMap()
{
}

void
TileMap::write(lisp::Writer& writer)
{
  writer.start_list("tilemap");

  writer.write("z-pos", z_pos);

  writer.write("solid", solid);
  writer.write("speed", speed_x);
  writer.write("speed-y", speed_y);
  writer.write("width", width);
  writer.write("height", height);
  writer.write("tiles", tiles);

  writer.end_list("tilemap");
}

void
TileMap::update(float elapsed_time)
{
  // handle tilemap fading
  if (current_alpha != alpha) {
    remaining_fade_time = std::max(0.0f, remaining_fade_time - elapsed_time);
    if (remaining_fade_time == 0.0f) {
      current_alpha = alpha;
    } else {
      float amt = (alpha - current_alpha) / (remaining_fade_time / elapsed_time);
      if (amt > 0) current_alpha = std::min(current_alpha + amt, alpha);
      if (amt < 0) current_alpha = std::max(current_alpha + amt, alpha);
    }
    if ((alpha < 0.25) && (current_alpha < 0.25)) set_solid(false);
    if ((alpha > 0.75) && (current_alpha > 0.75)) set_solid(true);
  }

  movement = Vector(0,0);
  // if we have a path to follow, follow it
  if (walker.get()) {
    Vector v = walker->advance(elapsed_time);
    movement = Vector(v.x-get_x_offset(), std::max(0.0f,v.y-get_y_offset()));
    set_x_offset(v.x);
    set_y_offset(v.y);
  }
}

void
TileMap::draw(DrawingContext& context)
{
  // skip draw if current opacity is set to 0.0
  if (current_alpha == 0.0) return;

  context.push_transform();
  context.push_target();
  context.set_target(draw_target);

  if(drawing_effect != 0) context.set_drawing_effect(drawing_effect);
  if(current_alpha != 1.0) context.set_alpha(current_alpha);

  float trans_x = roundf(context.get_translation().x);
  float trans_y = roundf(context.get_translation().y);
  context.set_translation(Vector(int(trans_x * speed_x),
                                 int(trans_y * speed_y)));

  /** if we don't round here, we'll have a 1 pixel gap on screen sometimes.
   * I have no idea why */
  float start_x = int((roundf(context.get_translation().x) - roundf(x_offset)) / 32) * 32 + roundf(x_offset);
  float start_y = int((roundf(context.get_translation().y) - roundf(y_offset)) / 32) * 32 + roundf(y_offset);
  float end_x = std::min(start_x + SCREEN_WIDTH + 32, float(width * 32 + roundf(x_offset)));
  float end_y = std::min(start_y + SCREEN_HEIGHT + 32, float(height * 32 + roundf(y_offset)));
  int tsx = int((start_x - roundf(x_offset)) / 32); // tilestartindex x
  int tsy = int((start_y - roundf(y_offset)) / 32); // tilestartindex y

  Vector pos;
  int tx, ty;
  for(pos.x = start_x, tx = tsx; pos.x < end_x; pos.x += 32, ++tx) {
    for(pos.y = start_y, ty = tsy; pos.y < end_y; pos.y += 32, ++ty) {
      if ((tx < 0) || (ty < 0)) continue;
      const Tile* tile = tileset->get(tiles[ty*width + tx]);
      assert(tile != 0);
      tile->draw(context, pos, z_pos);
    }
  }

  context.pop_target();
  context.pop_transform();
}

void
TileMap::goto_node(int node_no)
{
  if (!walker.get()) return;
  walker->goto_node(node_no);
}

void
TileMap::start_moving()
{
  if (!walker.get()) return;
  walker->start_moving();
}

void
TileMap::stop_moving()
{
  if (!walker.get()) return;
  walker->stop_moving();
}

void
TileMap::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty()) return;
  Scripting::TileMap* interface = new Scripting::TileMap(this);
  expose_object(vm, table_idx, interface, name, true);
}

void
TileMap::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty()) return;
  Scripting::unexpose_object(vm, table_idx, name);
}

void
TileMap::set(int newwidth, int newheight, const std::vector<unsigned int>&newt,
    int new_z_pos, bool newsolid)
{
  if(int(newt.size()) != newwidth * newheight)
    throw std::runtime_error("Wrong tilecount count.");

  width  = newwidth;
  height = newheight;

  tiles.resize(newt.size());
  tiles = newt;

  z_pos  = new_z_pos;
  solid  = newsolid;

  // make sure all tiles are loaded
  for(Tiles::iterator i = tiles.begin(); i != tiles.end(); ++i)
    tileset->get(*i);
}

void
TileMap::resize(int new_width, int new_height, int fill_id)
{
  if(new_width < width) {
    // remap tiles for new width
    for(int y = 0; y < height && y < new_height; ++y) {
      for(int x = 0; x < new_width; ++x) {
        tiles[y * new_width + x] = tiles[y * width + x];
      }
    }
  }

  tiles.resize(new_width * new_height, fill_id);

  if(new_width > width) {
    // remap tiles
    for(int y = std::min(height, new_height)-1; y >= 0; --y) {
      for(int x = new_width-1; x >= 0; --x) {
        if(x >= width) {
          tiles[y * new_width + x] = fill_id;
          continue;
        }

        tiles[y * new_width + x] = tiles[y * width + x];
      }
    }
  }

  height = new_height;
  width = new_width;
}

void
TileMap::set_solid(bool solid)
{
  this->solid = solid;
}

uint32_t
TileMap::get_tile_id(int x, int y) const
{
  if(x < 0 || x >= width || y < 0 || y >= height) {
    //log_warning << "tile outside tilemap requested" << std::endl;
    return 0;
  }

  return tiles[y*width + x];
}


const Tile*
TileMap::get_tile(int x, int y) const
{
  uint32_t id = get_tile_id(x, y);
  return tileset->get(id);
}

uint32_t
TileMap::get_tile_id_at(const Vector& pos) const
{
  return get_tile_id(int(pos.x - x_offset)/32, int(pos.y - y_offset)/32);
}

const Tile*
TileMap::get_tile_at(const Vector& pos) const
{
  uint32_t id = get_tile_id_at(pos);
  return tileset->get(id);
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
  change(int(pos.x - x_offset)/32, int(pos.y - y_offset)/32, newtile);
}

void
TileMap::change_all(uint32_t oldtile, uint32_t newtile)
{
  for (size_t x = 0; x < get_width(); x++) {
    for (size_t y = 0; y < get_height(); y++) {
      if (get_tile_id(x,y) != oldtile)
        continue;

      change(x,y,newtile);
    }
  }
}

void
TileMap::fade(float alpha, float seconds)
{
  this->alpha = alpha;
  this->remaining_fade_time = seconds;
}


void 
TileMap::set_alpha(float alpha)
{
  this->alpha = alpha;
  this->current_alpha = alpha;
  this->remaining_fade_time = 0;
  if (current_alpha < 0.25) set_solid(false);
  if (current_alpha > 0.75) set_solid(true);
}

float 
TileMap::get_alpha()
{
  return this->current_alpha;
}
  
IMPLEMENT_FACTORY(TileMap, "tilemap");
