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

#include <math.h>

#include "object/tilemap.hpp"
#include "scripting/squirrel_util.hpp"
#include "scripting/tilemap.hpp"
#include "supertux/globals.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/tile_set.hpp"
#include "util/reader.hpp"

TileMap::TileMap(const TileSet *new_tileset) :
  tileset(new_tileset),
  tiles(),
  real_solid(false),
  effective_solid(false),
  speed_x(1),
  speed_y(1),
  width(0),
  height(0),
  z_pos(0),
  offset(Vector(0,0)),
  movement(0,0),
  drawing_effect(NO_EFFECT),
  alpha(1.0),
  current_alpha(1.0),
  remaining_fade_time(0),
  path(),
  walker(),
  draw_target(DrawingContext::NORMAL)
{
}

TileMap::TileMap(const Reader& reader) :
  tileset(),
  tiles(),
  real_solid(false),
  effective_solid(false),
  speed_x(1),
  speed_y(1),
  width(-1),
  height(-1),
  z_pos(0),
  offset(Vector(0,0)),
  movement(Vector(0,0)),
  drawing_effect(NO_EFFECT),
  alpha(1.0),
  current_alpha(1.0),
  remaining_fade_time(0),
  path(),
  walker(),
  draw_target(DrawingContext::NORMAL)
{
  tileset = current_tileset;
  assert(tileset != NULL);

  reader.get("name",   name);
  reader.get("solid",  real_solid);
  reader.get("speed",  speed_x);
  reader.get("speed-y", speed_y);

  z_pos = reader_get_layer (reader, /* default = */ 0);

  if(real_solid && ((speed_x != 1) || (speed_y != 1))) {
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
    set_offset(v);
  }

  std::string draw_target_s = "normal";
  reader.get("draw-target", draw_target_s);
  if (draw_target_s == "normal") draw_target = DrawingContext::NORMAL;
  if (draw_target_s == "lightmap") draw_target = DrawingContext::LIGHTMAP;

  if (reader.get("alpha", alpha)) {
    current_alpha = alpha;
  }

  /* Initialize effective_solid based on real_solid and current_alpha. */
  effective_solid = real_solid;
  update_effective_solid ();

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
  {
    log_info << "Tilemap '" << name << "', z-pos '" << z_pos << "' is empty." << std::endl;
  }
}

TileMap::TileMap(const TileSet *new_tileset, std::string name_, int z_pos_,
                 bool solid, size_t width_, size_t height_) :
  tileset(new_tileset),
  tiles(),
  real_solid(solid),
  effective_solid(solid),
  speed_x(1),
  speed_y(1),
  width(0),
  height(0),
  z_pos(z_pos_),
  offset(Vector(0,0)),
  movement(Vector(0,0)),
  drawing_effect(NO_EFFECT),
  alpha(1.0),
  current_alpha(1.0),
  remaining_fade_time(0),
  path(),
  walker(),
  draw_target(DrawingContext::NORMAL)
{
  this->name = name_;

  if (this->z_pos > (LAYER_GUI - 100))
    this->z_pos = LAYER_GUI - 100;

  resize(width_, height_);
}

TileMap::~TileMap()
{
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
    update_effective_solid ();
  }

  movement = Vector(0,0);
  // if we have a path to follow, follow it
  if (walker.get()) {
    Vector v = walker->advance(elapsed_time);
    movement = v - get_offset();
    set_offset(v);
  }
}

void
TileMap::draw(DrawingContext& context)
{
  // skip draw if current opacity is 0.0
  if (current_alpha == 0.0) return;

  context.push_transform();
  if(draw_target != DrawingContext::NORMAL) {
    context.push_target();
    context.set_target(draw_target);
  }

  if(drawing_effect != 0) context.set_drawing_effect(drawing_effect);
  if(current_alpha != 1.0) context.set_alpha(current_alpha);

  /* Force the translation to be an integer so that the tiles appear sharper.
   * For consistency (i.e., to avoid 1-pixel gaps), this needs to be done even
   * for solid tilemaps that are guaranteed to have speed 1.
   * FIXME Force integer translation for all graphics, not just tilemaps. */
  float trans_x = roundf(context.get_translation().x);
  float trans_y = roundf(context.get_translation().y);
  context.set_translation(Vector(int(trans_x * speed_x),
                                 int(trans_y * speed_y)));

  Rectf draw_rect = Rectf(context.get_translation(),
        context.get_translation() + Vector(SCREEN_WIDTH, SCREEN_HEIGHT));
  Rect t_draw_rect = get_tiles_overlapping(draw_rect);
  Vector start = get_tile_position(t_draw_rect.left, t_draw_rect.top);

  Vector pos;
  int tx, ty;

  for(pos.x = start.x, tx = t_draw_rect.left; tx < t_draw_rect.right; pos.x += 32, ++tx) {
    for(pos.y = start.y, ty = t_draw_rect.top; ty < t_draw_rect.bottom; pos.y += 32, ++ty) {
      int index = ty*width + tx;
      assert (index >= 0);
      assert (index < (width * height));

      if (tiles[index] == 0) continue;
      const Tile* tile = tileset->get(tiles[index]);
      assert(tile != 0);
      tile->draw(context, pos, z_pos);
    } /* for (pos y) */
  } /* for (pos x) */

  if(draw_target != DrawingContext::NORMAL) {
    context.pop_target();
  }
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
  scripting::TileMap* _this = new scripting::TileMap(this);
  expose_object(vm, table_idx, _this, name, true);
}

void
TileMap::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty()) return;
  scripting::unexpose_object(vm, table_idx, name);
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

  if (new_z_pos > (LAYER_GUI - 100))
    z_pos = LAYER_GUI - 100;
  else
    z_pos  = new_z_pos;
  real_solid  = newsolid;
  update_effective_solid ();

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

Rect
TileMap::get_tiles_overlapping(const Rectf &rect) const
{
  Rectf rect2 = rect;
  rect2.move(-offset);

  int t_left   = std::max(0     , int(floorf(rect2.get_left  () / 32)));
  int t_right  = std::min(width , int(ceilf (rect2.get_right () / 32)));
  int t_top    = std::max(0     , int(floorf(rect2.get_top   () / 32)));
  int t_bottom = std::min(height, int(ceilf (rect2.get_bottom() / 32)));
  return Rect(t_left, t_top, t_right, t_bottom);
}

void
TileMap::set_solid(bool solid)
{
  this->real_solid = solid;
  update_effective_solid ();
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
  Vector xy = (pos - offset) / 32;
  return get_tile_id(int(xy.x), int(xy.y));
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
  Vector xy = (pos - offset) / 32;
  change(int(xy.x), int(xy.y), newtile);
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
TileMap::fade(float alpha_, float seconds)
{
  this->alpha = alpha_;
  this->remaining_fade_time = seconds;
}

void
TileMap::set_alpha(float alpha_)
{
  this->alpha = alpha_;
  this->current_alpha = alpha;
  this->remaining_fade_time = 0;
  update_effective_solid ();
}

float
TileMap::get_alpha()
{
  return this->current_alpha;
}

/*
 * Private methods
 */
void
TileMap::update_effective_solid (void)
{
  if (!real_solid)
    effective_solid = false;
  else if (effective_solid && (current_alpha < .25))
    effective_solid = false;
  else if (!effective_solid && (current_alpha >= .75))
    effective_solid = true;
}

/* EOF */
