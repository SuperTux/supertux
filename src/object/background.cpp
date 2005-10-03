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

#include "background.hpp"
#include "camera.hpp"
#include "video/drawing_context.hpp"
#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"
#include "object_factory.hpp"
#include "resources.hpp"
#include "main.hpp"

Background::Background()
  : type(INVALID), layer(LAYER_BACKGROUND0), image(0)
{
}

Background::Background(const lisp::Lisp& reader)
  : type(INVALID), layer(LAYER_BACKGROUND0), image(0)
{
  reader.get("layer", layer);
  if(reader.get("image", imagefile) 
      && reader.get("speed", speed)) {
    set_image(imagefile, speed);
  } else {
    std::vector<float> bkgd_top_color, bkgd_bottom_color;
    if(reader.get_vector("top_color", bkgd_top_color) &&
        reader.get_vector("bottom_color", bkgd_bottom_color))
      set_gradient(Color(bkgd_top_color),
                   Color(bkgd_bottom_color));
  }
}

Background::~Background()
{
  delete image;
}

void
Background::write(lisp::Writer& writer)
{
  if(type == INVALID)
    return;
    
  writer.start_list("background");

  if(type == IMAGE) {
    writer.write_string("image", imagefile);
    writer.write_float("speed", speed);
  } else if(type == GRADIENT) {
    std::vector<float> bkgd_top_color, bkgd_bottom_color;
    bkgd_top_color.push_back(gradient_top.red);
    bkgd_top_color.push_back(gradient_top.green);
    bkgd_top_color.push_back(gradient_top.blue);
    bkgd_bottom_color.push_back(gradient_top.red);
    bkgd_bottom_color.push_back(gradient_top.green);
    bkgd_bottom_color.push_back(gradient_top.blue);
    writer.write_float_vector("top_color", bkgd_top_color);
    writer.write_float_vector("bottom_color", bkgd_bottom_color);
  }
  writer.write_int("layer", layer);
  
  writer.end_list("background");
}

void
Background::update(float)
{
}

void
Background::set_image(const std::string& name, float speed)
{
  this->type = IMAGE;
  this->imagefile = name;
  this->speed = speed;

  delete image;
  image = new Surface("images/background/" + name);
}

void
Background::set_gradient(Color top, Color bottom)
{
  type = GRADIENT;
  gradient_top = top;
  gradient_bottom = bottom;

  delete image;
  image = NULL;
}

void
Background::draw(DrawingContext& context)
{
  if(type == GRADIENT) {
    context.push_transform();
    context.set_translation(Vector(0, 0));
    context.draw_gradient(gradient_top, gradient_bottom, layer);
    context.pop_transform();
  } else if(type == IMAGE) {
    if(!image)
      return;
    
    int w = (int) image->get_width();
    int h = (int) image->get_height();
    int sx = int(-context.get_translation().x * speed) % w - w;
    int sy = int(-context.get_translation().y * speed) % h - h;
    context.push_transform();
    context.set_translation(Vector(0, 0));
    for(int x = sx; x < SCREEN_WIDTH; x += w)
      for(int y = sy; y < SCREEN_HEIGHT; y += h)
        context.draw_surface(image, Vector(x, y), layer);
    context.pop_transform();
  }
}

IMPLEMENT_FACTORY(Background, "background");
