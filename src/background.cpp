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

#include "background.h"
#include "app/globals.h"
#include "camera.h"
#include "video/drawing_context.h"
#include "utils/lispwriter.h"

Background::Background()
  : type(INVALID), layer(LAYER_BACKGROUND0), image(0)
{
}

Background::Background(LispReader& reader)
  : type(INVALID), layer(LAYER_BACKGROUND0), image(0)
{
  reader.read_int("layer", layer);
  if(reader.read_string("image", imagefile) 
      && reader.read_float("speed", speed)) {
    set_image(imagefile, speed);
  }

  std::vector <unsigned int> bkgd_top_color, bkgd_bottom_color;
  if(reader.read_int_vector("top_color", bkgd_top_color) &&
     reader.read_int_vector("bottom_color", bkgd_bottom_color))
    set_gradient(Color(bkgd_top_color), Color(bkgd_bottom_color));
}

Background::~Background()
{
  printf("bgfree.\n");
  delete image;
}

void
Background::write(LispWriter& writer)
{
  if(type == INVALID)
    return;
    
  writer.start_list("background");

  if(type == IMAGE) {
    writer.write_string("image", imagefile);
    writer.write_float("speed", speed);
  } else if(type == GRADIENT) {
    std::vector <unsigned int> bkgd_top_color, bkgd_bottom_color;
    bkgd_top_color.push_back(gradient_top.red);
    bkgd_top_color.push_back(gradient_top.green);
    bkgd_top_color.push_back(gradient_top.blue);
    bkgd_bottom_color.push_back(gradient_top.red);
    bkgd_bottom_color.push_back(gradient_top.green);
    bkgd_bottom_color.push_back(gradient_top.blue);
    writer.write_int_vector("top_color", bkgd_top_color);
    writer.write_int_vector("bottom_color", bkgd_bottom_color);
  }
  writer.write_int("layer", layer);
  
  writer.end_list("background");
}

void
Background::action(float)
{
}

void
Background::set_image(const std::string& name, float speed)
{
  this->type = IMAGE;
  this->imagefile = name;
  this->speed = speed;

  printf("seti %p\n", this);
  delete image;
  image = new Surface(datadir + "/images/background/" + name, false);
}

void
Background::set_gradient(Color top, Color bottom)
{
  type = GRADIENT;
  gradient_top = top;
  gradient_bottom = bottom;

  delete image;
  image = new Surface(top, bottom, screen->w, screen->h);
}

void
Background::draw(DrawingContext& context)
{
  if(type == GRADIENT) {
    context.push_transform();
    context.set_translation(Vector(0, 0));
    context.draw_surface(image, Vector(0, 0), layer);
    context.pop_transform();
  } else if(type == IMAGE) {
    if(!image)
      return;
    
    int sx = int(-context.get_translation().x * speed) % image->w - image->w;
    int sy = int(-context.get_translation().y * speed) % image->h - image->h;
    context.push_transform();
    context.set_translation(Vector(0, 0));
    for(int x = sx; x < screen->w; x += image->w)
      for(int y = sy; y < screen->h; y += image->h)
        context.draw_surface(image, Vector(x, y), layer);
    context.pop_transform();
  }
}

