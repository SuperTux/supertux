//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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

#include <cmath>
#include <cassert>
#include <iostream>
#include <stdexcept>

#include "app/globals.h"
#include "tile.h"
#include "scene.h"
#include "resources.h"
#include "utils/lispreader.h"
#include "math/vector.h"
#include "video/drawing_context.h"

Tile::Tile()
  : id(0), editor_image(0), attributes(0), data(0), anim_fps(1)
{
}

Tile::~Tile()
{
  for(std::vector<Surface*>::iterator i = images.begin(); i != images.end();
      ++i) {
    delete *i;
  }
  delete editor_image;
}

void
Tile::parse(LispReader& reader)
{
  if(!reader.read_uint("id", id)) {
    throw std::runtime_error("Missing tile-id.");
  }
  
  bool value;
  if(reader.read_bool("solid", value) && value)
    attributes |= SOLID;
  if(reader.read_bool("unisolid", value) && value)
    attributes |= UNISOLID | SOLID;
  if(reader.read_bool("brick", value) && value)
    attributes |= BRICK;
  if(reader.read_bool("ice", value) && value)
    attributes |= ICE;
  if(reader.read_bool("water", value) && value)
    attributes |= WATER;
  if(reader.read_bool("spike", value) && value)
    attributes |= SPIKE;
  if(reader.read_bool("fullbox", value) && value)
    attributes |= FULLBOX;
  if(reader.read_bool("distro", value) && value)
    attributes |= COIN;
  if(reader.read_bool("coin", value) && value)
    attributes |= COIN;
  if(reader.read_bool("goal", value) && value)
    attributes |= GOAL;

  reader.read_int("data", data);
  reader.read_float("anim-fps", anim_fps);

  if(reader.read_int("slope-type", data)) {
    attributes |= SOLID | SLOPE;
  }

  parse_images(reader.read_lisp("images"));
  reader.read_string("editor-images", editor_imagefile);
}

void
Tile::parse_images(lisp_object_t* list)
{
  while(!lisp_nil_p(list)) {
    lisp_object_t* cur = lisp_car(list);
    if(lisp_string_p(cur)) {
      imagespecs.push_back(ImageSpec(lisp_string(cur), Rectangle(0, 0, 0, 0)));
    } else if(lisp_cons_p(cur) && lisp_symbol_p(lisp_car(cur))) {
      lisp_object_t* sym  = lisp_car(cur);
      lisp_object_t* data = lisp_cdr(cur);
      
      if (strcmp(lisp_symbol(sym), "region") == 0) {
        float x = lisp_integer(lisp_list_nth(data, 1));
        float y = lisp_integer(lisp_list_nth(data, 2));
        float width = lisp_integer(lisp_list_nth(data, 3));
        float height = lisp_integer(lisp_list_nth(data, 4));
        imagespecs.push_back(ImageSpec(lisp_string(lisp_car(data)),
              Rectangle(x, y, x+width, y+height)));
      } else {
        std::cerr << "Tile: Type mismatch, should be '(region \"somestring\" x y w h)'" << std::endl;
        continue;
      }
    } else {
      std::cerr << "Expected string or list in images tag.\n";
      continue;
    }
    
    list = lisp_cdr(list);
  }
}

void
Tile::load_images()
{
  assert(images.size() == 0);
  for(std::vector<ImageSpec>::iterator i = imagespecs.begin(); i !=
      imagespecs.end(); ++i) {
    const ImageSpec& spec = *i;
    Surface* surface;
    std::string file 
      = get_resource_filename(std::string("images/tilesets/") + spec.file);
    if(spec.rect.get_width() <= 0) {
      surface = new Surface(file, true);
    } else {
      surface = new Surface(file,
          (int) spec.rect.p1.x,
          (int) spec.rect.p1.y,
          (int) spec.rect.get_width(),
          (int) spec.rect.get_height(), true);
    }
    images.push_back(surface);
  }
  if(editor_imagefile != "") {
    editor_image = new Surface(
        get_resource_filename(
          std::string("images/tilesets/") + editor_imagefile), true);
  }
}

Surface*
Tile::get_editor_image() const
{
  if(editor_image)
    return editor_image;
  if(images.size() > 0)
    return images[0];

  return 0;
}

void
Tile::draw(DrawingContext& context, const Vector& pos, int layer) const
{
  if(images.size() > 1) {
    size_t frame = size_t(global_time * anim_fps) % images.size();
    context.draw_surface(images[frame], pos, layer);
  } else if (images.size() == 1) {
    context.draw_surface(images[0], pos, layer);
  }
}

