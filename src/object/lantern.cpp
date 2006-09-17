//  $Id$
//
//  SuperTux - Lantern
//  Copyright (C) 2006 Wolfgang Becker <uafr@gmx.de>
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

#include "lantern.hpp"
#include "sprite/sprite_manager.hpp"
#include "object_factory.hpp"

Lantern::Lantern(const lisp::Lisp& reader)
  : Rock(reader, "images/objects/lantern/lantern.sprite"),
    lightcolor(1.0f, 1.0f, 1.0f)
{
  //get color from lisp
  std::vector<float> vColor;
  reader.get_vector("color", vColor);
  lightcolor = Color(vColor);
  lightsprite = sprite_manager->create("images/objects/lightmap_light/lightmap_light.sprite");
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  updateColor();
}

Lantern::~Lantern()
{
  delete lightsprite;
}

void
Lantern::updateColor(){
  lightsprite->set_color(lightcolor);
  //Turn lantern off if light is black
  if(lightcolor.red == 0 && lightcolor.green == 0 && lightcolor.blue == 0){
     sprite->set_action("off");
  } else {
     sprite->set_action("normal");
  }
}

void
Lantern::draw(DrawingContext& context){
  //Draw the Sprite.
  MovingSprite::draw(context);
  //Let there be light.
  context.push_target();
  context.set_target(DrawingContext::LIGHTMAP);

  lightsprite->draw(context, get_bbox().get_middle(), 0);

  context.pop_target();
}

IMPLEMENT_FACTORY(Lantern, "lantern");
