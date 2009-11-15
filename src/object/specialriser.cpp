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

//#include <config.h>

//#include <math.h>

#include "object/camera.hpp"
#include "object/specialriser.hpp"
//#include "sprite/sprite_manager.hpp"
//#include "supertux/resources.hpp"
#include "supertux/sector.hpp"

SpecialRiser::SpecialRiser(Vector pos, MovingObject* _child)
  : child(_child)
{
  _child->set_pos(pos - Vector(0, 32));
  offset = 0;
}

SpecialRiser::~SpecialRiser()
{
}

void
SpecialRiser::update(float elapsed_time)
{
  offset += 50 * elapsed_time;
  if(offset > 32) {
    Sector::current()->add_object(child);
    remove_me();
  }
}

void
SpecialRiser::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(
      context.get_translation() + Vector(0, -32 + offset));
  child->draw(context);
  context.pop_transform();
}
