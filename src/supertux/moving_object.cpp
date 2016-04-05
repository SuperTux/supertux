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

#include "supertux/moving_object.hpp"

#include "editor/resizer.hpp"
#include "supertux/sector.hpp"

MovingObject::MovingObject() :
  bbox(),
  movement(),
  group(COLGROUP_MOVING),
  dest()
{
}

MovingObject::~MovingObject()
{
}

void
MovingObject::save(Writer& writer) {
  GameObject::save(writer);
  writer.write("x", bbox.p1.x);
  writer.write("y", bbox.p1.y);
}

void
MovingObject::edit_bbox() {
  GameObjectPtr marker1, marker2, marker3, marker4, marker5, marker6, marker7, marker8;
  marker1 = std::make_shared<Resizer>(&bbox, Resizer::LEFT_UP, Resizer::LEFT_UP);
  marker2 = std::make_shared<Resizer>(&bbox, Resizer::LEFT_UP, Resizer::NONE);
  marker3 = std::make_shared<Resizer>(&bbox, Resizer::LEFT_UP, Resizer::RIGHT_DOWN);
  marker4 = std::make_shared<Resizer>(&bbox, Resizer::NONE, Resizer::LEFT_UP);
  marker5 = std::make_shared<Resizer>(&bbox, Resizer::NONE, Resizer::RIGHT_DOWN);
  marker6 = std::make_shared<Resizer>(&bbox, Resizer::RIGHT_DOWN, Resizer::LEFT_UP);
  marker7 = std::make_shared<Resizer>(&bbox, Resizer::RIGHT_DOWN, Resizer::NONE);
  marker8 = std::make_shared<Resizer>(&bbox, Resizer::RIGHT_DOWN, Resizer::RIGHT_DOWN);
  Sector::current()->add_object(marker1);
  Sector::current()->add_object(marker2);
  Sector::current()->add_object(marker3);
  Sector::current()->add_object(marker4);
  Sector::current()->add_object(marker5);
  Sector::current()->add_object(marker6);
  Sector::current()->add_object(marker7);
  Sector::current()->add_object(marker8);
}

/* EOF */
