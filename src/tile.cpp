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
#include "lisp/lisp.h"
#include "tile.h"
#include "scene.h"
#include "resources.h"
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
Tile::parse(const lisp::Lisp& reader)
{
  if(!reader.get("id", id)) {
    throw std::runtime_error("Missing tile-id.");
  }
  
  bool value;
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
  if(reader.get("spike", value) && value)
    attributes |= SPIKE;
  if(reader.get("fullbox", value) && value)
    attributes |= FULLBOX;
  if(reader.get("distro", value) && value)
    attributes |= COIN;
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
  reader.get("editor-images", editor_imagefile);
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
      imagespecs.push_back(ImageSpec(file, Rectangle(0, 0, 0, 0)));
    } else if(cur->get_type() == lisp::Lisp::TYPE_CONS && 
        cur->get_car()->get_type() == lisp::Lisp::TYPE_SYMBOL) {
      const lisp::Lisp* ptr = cur->get_cdr();

      std::string file;
      float x, y, w, h;
      ptr->get_car()->get(file); ptr = ptr->get_cdr();
      ptr->get_car()->get(x); ptr = ptr->get_cdr();
      ptr->get_car()->get(y); ptr = ptr->get_cdr();
      ptr->get_car()->get(w); ptr = ptr->get_cdr();
      ptr->get_car()->get(h);
      imagespecs.push_back(ImageSpec(file, Rectangle(x, y, x+w, y+h)));
    } else {
      std::cerr << "Expected string or list in images tag.\n";
      continue;
    }
    
    list = list->get_cdr();
  }
}

void
Tile::load_images(const std::string& tilesetpath)
{
  assert(images.size() == 0);
  for(std::vector<ImageSpec>::iterator i = imagespecs.begin(); i !=
      imagespecs.end(); ++i) {
    const ImageSpec& spec = *i;
    Surface* surface;
    std::string file 
      = get_resource_filename(tilesetpath + spec.file);
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

