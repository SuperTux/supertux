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
#include "display_manager.h"

Background::Background(DisplayManager& displaymanager)
{
  displaymanager.add_drawable(this, LAYER_BACKGROUND0);
}

Background::~Background()
{
}

void
Background::action(float)
{
}

void
Background::set_image(Surface* image, float speed)
{
  bgtype = BACKGROUND_IMAGE;
  this->image = image;
  this->speed = speed;
}

void
Background::set_gradient(Color top, Color bottom)
{
  bgtype = BACKGROUND_GRADIENT;
  gradient_top = top;
  gradient_bottom = bottom;
}

void
Background::draw(Camera& viewport, int )
{
  if(bgtype == BACKGROUND_GRADIENT) {
    drawgradient(gradient_top, gradient_bottom);
  } else if(bgtype == BACKGROUND_IMAGE) {
    int sx = int(-viewport.get_translation().x * float(speed/100.))
      % image->w - image->w;
    int sy = int(-viewport.get_translation().y * float(speed/100.))
      % image->h - image->h;
    for(int x = sx; x < screen->w; x += image->w)
      for(int y = sy; y < screen->h; y += image->h)
        image->draw(x, y);
  }
}

