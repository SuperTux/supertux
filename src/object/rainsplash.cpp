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
#include <config.h>

#include "object/rainsplash.hpp"
#include "supertux/sector.hpp"

RainSplash::RainSplash(Vector pos, bool vertical)
{
  frame = 0;
  position = pos;
  if (vertical) sprite = sprite_manager->create("images/objects/particles/rainsplash-vertical.sprite");
  else sprite = sprite_manager->create("images/objects/particles/rainsplash.sprite");
}

RainSplash::~RainSplash() {
  remove_me();
}

void
RainSplash::hit(Player& )
{
}

void
RainSplash::update(float time)
{
  time = 0;//just so i don't get an "unused variable" error - don't know how to circumvent this
  frame++;
  if (frame >= 10) remove_me();
}

void
RainSplash::draw(DrawingContext& context)
{
   sprite->draw(context, position, LAYER_OBJECTS);
}
