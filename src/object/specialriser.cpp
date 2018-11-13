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

#include "object/specialriser.hpp"

#include "supertux/sector.hpp"
#include "video/drawing_context.hpp"

SpecialRiser::SpecialRiser(const Vector& pos, std::unique_ptr<MovingObject> child_) :
  offset(0),
  child(std::move(child_))
{
  child->set_pos(pos - Vector(0, 32));
}

void
SpecialRiser::update(float dt_sec)
{
  offset += 50 * dt_sec;
  if (offset > 32) {
    Sector::get().add_object(std::move(child));
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

/* EOF */
