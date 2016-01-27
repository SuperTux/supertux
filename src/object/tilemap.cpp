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

#include "object/tilemap.hpp"

#include <math.h>

#include "scripting/squirrel_util.hpp"
#include "scripting/tilemap.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/tile_set.hpp"
#include "util/reader.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

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
  tint(1, 1, 1),
  current_tint(1, 1, 1),
  remaining_tint_fade_time(0),
  path(),
  walker(),
  draw_target(DrawingContext::NORMAL)
{
}

TileMap::TileMap(const TileSet *tileset_, const ReaderMapping& reader) :
  tileset(tileset_),
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
  tint(1, 1, 1),
  current_tint(1, 1, 1),
  remaining_tint_fade_time(0),
  path(),
  walker(),
  draw_target(DrawingContext::NORMAL)
{
  assert(tileset);

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

  ReaderMapping path_mapping;
  if (reader.get("path", path_mapping)) {
    path.reset(new Path());
    path->read(path_mapping);
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

  std::vector<float> vColor;
  if (reader.get("tint", vColor)) {
    current_tint = Color(vColor);
    tint = current_tint;
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
  for(Tiles::const_iterator i = tiles.begin(); i != tiles.end(); ++i) {
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

TileMap::~TileMap()
{
}

void TileMap::float_channel(float target, float &current, float remaining_time, float elapsed_time)
{
  float amt = (target - current) / (remaining_time / elapsed_time);
  if (amt > 0) current = std::min(current + amt, target);
  if (amt < 0) current = std::max(current + amt, target);
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
      float_channel(alpha, current_alpha, remaining_fade_time, elapsed_time);
    }
    update_effective_solid ();
  }

  // handle tint fading
  if (current_tint.red != tint.red || current_tint.green != tint.green ||
      current_tint.blue != tint.blue || current_tint.alpha != tint.alpha) {

    remaining_tint_fade_time = std::max(0.0f, remaining_tint_fade_time - elapsed_time);
    if (remaining_tint_fade_time == 0.0f) {
      current_tint = tint;
    } else {
      float_channel(tint.red  , current_tint.red  , remaining_tint_fade_time, elapsed_time);
      float_channel(tint.green, current_tint.green, remaining_tint_fade_time, elapsed_time);
      float_channel(tint.blue , current_tint.blue , remaining_tint_fade_time, elapsed_time);
      float_channel(tint.alpha, current_tint.alpha, remaining_tint_fade_time, elapsed_time);
    }
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

  // Make sure the tilemap is within draw view
  if (t_draw_rect.is_valid()) {
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

        tile->draw(context, pos, z_pos, current_tint);
      } /* for (pos y) */
    } /* for (pos x) */

    /* Make sure that tiles with images larger than 32x32 that overlap
     * the draw rect will be drawn, even if their tile position does
     * not fall within the draw rect. */
    static const int EXTENDING_TILES = 32;
    int ex_left = std::max(0, t_draw_rect.left-EXTENDING_TILES);
    int ex_top = std::max(0, t_draw_rect.top-EXTENDING_TILES);
    Vector ex_start = get_tile_position(ex_left, ex_top);

    for (pos.x = start.x, tx = t_draw_rect.left; tx < t_draw_rect.right; pos.x += 32, ++tx) {
      for (pos.y = ex_start.y, ty = ex_top; ty < t_draw_rect.top; pos.y += 32, ++ty) {
        int index = ty*width + tx;
        assert (index >= 0);
        assert (index < (width * height));

        if (tiles[index] == 0) continue;
        const Tile* tile = tileset->get(tiles[index]);
        assert(tile != 0);

        SurfacePtr image = tile->get_current_image();
        if (image) {
          int h = image->get_height();
          if (h <= 32) continue;

          if (pos.y + h > start.y)
            tile->draw(context, pos, z_pos, current_tint);
        }
      }
    }

    for (pos.x = ex_start.x, tx = ex_left; tx < t_draw_rect.right; pos.x += 32, ++tx) {
      for(pos.y = ex_start.y, ty = ex_top; ty < t_draw_rect.bottom; pos.y += 32, ++ty) {
        int index = ty*width + tx;
        assert (index >= 0);
        assert (index < (width * height));

        if (tiles[index] == 0) continue;
        const Tile* tile = tileset->get(tiles[index]);
        assert(tile != 0);

        SurfacePtr image = tile->get_current_image();
        if (image) {
          int w = image->get_width();
          int h = image->get_height();
          if (w <= 32 && h <= 32) continue;

          if (pos.x + w > start.x && pos.y + h > start.y)
            tile->draw(context, pos, z_pos, current_tint);
        }
      }
    }
  }

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
  for(Tiles::const_iterator i = tiles.begin(); i != tiles.end(); ++i)
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
TileMap::tint_fade(Color new_tint, float seconds)
{
  this->tint = new_tint;
  this->remaining_tint_fade_time = seconds;
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
TileMap::get_alpha() const
{
  return this->current_alpha;
}

/*
 * Private methods
 */
void
TileMap::update_effective_solid()
{
  if (!real_solid)
    effective_solid = false;
  else if (effective_solid && (current_alpha < .25))
    effective_solid = false;
  else if (!effective_solid && (current_alpha >= .75))
    effective_solid = true;
}

/* EOF */
