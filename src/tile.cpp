//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include <config.h>

#include "tile.hpp"

#include <math.h>
#include <assert.h>
#include <iostream>
#include <stdexcept>

#include "lisp/lisp.hpp"
#include "tile_set.hpp"
#include "timer.hpp"
#include "math/vector.hpp"
#include "video/drawing_context.hpp"
#include "log.hpp"


Tile::Tile(const TileSet *new_tileset)
  : tileset(new_tileset), attributes(0), data(0), anim_fps(1)
{
}

Tile::Tile(const TileSet *new_tileset, std::vector<std::string> images, Rect rect, Uint32 attributes, Uint32 data, float animfps)
  : tileset(new_tileset), attributes(attributes), data(data), anim_fps(animfps)
{
  for(std::vector<std::string>::iterator i = images.begin(); i != images.end(); ++i) {
      imagespecs.push_back(ImageSpec(*i, rect));
  }
  correct_attributes();
}

Tile::~Tile()
{
  for(std::vector<Surface*>::iterator i = images.begin(); i != images.end();
      ++i) {
    delete *i;
  }
}

uint32_t
Tile::parse(const lisp::Lisp& reader)
{
  uint32_t id;
  if(!reader.get("id", id)) {
    throw std::runtime_error("Missing tile-id.");
  }

  bool value = false;
  if(reader.get("solid", value) && value)
    attributes |= SOLID;
  if(reader.get("unisolid", value) && value)
    attributes |= UNISOLID | SOLID;
  if(reader.get("brick", value) && value)
    attributes |= BRICK;
  if(reader.get("ice", value) && value)
    attributes |= ICE;
  if(reader.get("water", value) && value)
    attributes |= WATER;
  if(reader.get("hurts", value) && value)
    attributes |= HURTS;
  if(reader.get("fire", value) && value)
    attributes |= FIRE;
  if(reader.get("fullbox", value) && value)
    attributes |= FULLBOX;
  if(reader.get("coin", value) && value)
    attributes |= COIN;
  if(reader.get("goal", value) && value)
    attributes |= GOAL;

  if(reader.get("north", value) && value)
    data |= WORLDMAP_NORTH;
  if(reader.get("south", value) && value)
    data |= WORLDMAP_SOUTH;
  if(reader.get("west", value) && value)
    data |= WORLDMAP_WEST;
  if(reader.get("east", value) && value)
    data |= WORLDMAP_EAST;
  if(reader.get("stop", value) && value)
    data |= WORLDMAP_STOP;

  reader.get("data", data);
  reader.get("anim-fps", anim_fps);

  if(reader.get("slope-type", data)) {
    attributes |= SOLID | SLOPE;
  }

  const lisp::Lisp* images = reader.get_lisp("images");
  if(images)
    parse_images(*images);

  correct_attributes();
  return id;
}

void
Tile::parse_images(const lisp::Lisp& images_lisp)
{
  const lisp::Lisp* list = &images_lisp;
  while(list) {
    const lisp::Lisp* cur = list->get_car();
    if(cur->get_type() == lisp::Lisp::TYPE_STRING) {
      std::string file;
      cur->get(file);
      imagespecs.push_back(ImageSpec(file, Rect(0, 0, 0, 0)));
    } else if(cur->get_type() == lisp::Lisp::TYPE_CONS &&
              cur->get_car()->get_type() == lisp::Lisp::TYPE_SYMBOL &&
              cur->get_car()->get_symbol() == "region") {
      const lisp::Lisp* ptr = cur->get_cdr();

      std::string file;
      float x = 0, y = 0, w = 0, h = 0;
      ptr->get_car()->get(file); ptr = ptr->get_cdr();
      ptr->get_car()->get(x); ptr = ptr->get_cdr();
      ptr->get_car()->get(y); ptr = ptr->get_cdr();
      ptr->get_car()->get(w); ptr = ptr->get_cdr();
      ptr->get_car()->get(h);
      imagespecs.push_back(ImageSpec(file, Rect(x, y, x+w, y+h)));
    } else {
      log_warning << "Expected string or list in images tag" << std::endl;
      continue;
    }

    list = list->get_cdr();
  }
}

void
Tile::load_images()
{
  const std::string& tiles_path = tileset->tiles_path;

  assert(images.size() == 0);
  for(std::vector<ImageSpec>::iterator i = imagespecs.begin(); i !=
      imagespecs.end(); ++i) {
    const ImageSpec& spec = *i;
    Surface* surface;
    std::string file = tiles_path + spec.file;
    if(spec.rect.get_width() <= 0) {
      surface = new Surface(file);
    } else {
      surface = new Surface(file,
          (int) spec.rect.p1.x,
          (int) spec.rect.p1.y,
          (int) spec.rect.get_width(),
          (int) spec.rect.get_height());
    }
    images.push_back(surface);
  }
}

void
Tile::draw(DrawingContext& context, const Vector& pos, int z_pos) const
{
  if(images.size() > 1) {
    size_t frame = size_t(game_time * anim_fps) % images.size();
    context.draw_surface(images[frame], pos, z_pos);
  } else if (images.size() == 1) {
    context.draw_surface(images[0], pos, z_pos);
  }
}

void Tile::correct_attributes()
{
  //Fix little oddities in attributes (not many, currently...)
  if(!(attributes & SOLID) && (attributes & SLOPE || attributes & UNISOLID)) {
    attributes |= SOLID;
    //But still be vocal about it
    log_warning << "Tile with image " << imagespecs[0].file << " needs solid attribute." << std::endl;
  }
}
