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

Background::Background()
  : type(INVALID), image(0)
{
}

Background::~Background()
{
  delete image;
}

void
Background::action(float)
{
}

void
Background::set_image(const std::string& name, float speed)
{
  type = IMAGE;
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
}

void
Background::draw(DrawingContext& context)
{
  if(type == GRADIENT) {
    context.draw_gradient(gradient_top, gradient_bottom, LAYER_BACKGROUND0);
  } else if(type == IMAGE) {
    int sx = int(-context.get_translation().x * float(speed/100.))
      % image->w - image->w;
    int sy = int(-context.get_translation().y * float(speed/100.))
      % image->h - image->h;
    context.push_transform();
    context.set_translation(Vector(0, 0));
    for(int x = sx; x < screen->w; x += image->w)
      for(int y = sy; y < screen->h; y += image->h)
        context.draw_surface(image, Vector(x, y), LAYER_BACKGROUND0);
    context.pop_transform();
  }
}

