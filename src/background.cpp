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

#include "background.h"
#include "globals.h"
#include "camera.h"
#include "screen/drawing_context.h"
#include "lispwriter.h"

Background::Background()
  : type(INVALID), image(0)
{
}

Background::Background(LispReader& reader)
  : type(INVALID), image(0)
{
  if(reader.read_string("image", imagefile) 
      && reader.read_float("speed", speed)) {
    set_image(imagefile, speed);
  }

  int tr, tg, tb, br, bg, bb;
  if(reader.read_int("top_red", tr) && reader.read_int("top_green", tg)
      && reader.read_int("top_blue", tb) && reader.read_int("bottom_red", br)
      && reader.read_int("bottom_green", br)
      && reader.read_int("bottom_blue", bb)) {
    set_gradient(Color(tr, tg, tb), Color(br, bg, bb));
  }
}

Background::~Background()
{
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
    writer.write_int("top_red", gradient_top.red);
    writer.write_int("top_green", gradient_top.green);
    writer.write_int("top_blue", gradient_top.blue);
    writer.write_int("bottom_red", gradient_bottom.red);
    writer.write_int("bottom_green", gradient_bottom.green);
    writer.write_int("bottom_blue", gradient_bottom.blue);
  }
  
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

  delete image;
  image = new Surface(datadir + "/images/background/" + name, IGNORE_ALPHA);
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
    context.draw_surface(image, Vector(0, 0), LAYER_BACKGROUND0);
//    context.draw_gradient(gradient_top, gradient_bottom, LAYER_BACKGROUND0);
  } else if(type == IMAGE) {
    int sx = int(-context.get_translation().x * speed)
      % image->w - image->w;
    int sy = int(-context.get_translation().y * speed)
      % image->h - image->h;
    context.push_transform();
    context.set_translation(Vector(0, 0));
    for(int x = sx; x < screen->w; x += image->w)
      for(int y = sy; y < screen->h; y += image->h)
        context.draw_surface(image, Vector(x, y), LAYER_BACKGROUND0);
    context.pop_transform();
  }
}

