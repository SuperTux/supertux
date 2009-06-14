//  $Id$
//
//  SuperTux - Walking Leaf
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

#include "walkingleaf.hpp"

#include "random_generator.hpp"
#include "object/sprite_particle.hpp"
#include "object_factory.hpp"

WalkingLeaf::WalkingLeaf(const lisp::Lisp& reader)
        : WalkingBadguy(reader, "images/creatures/walkingleaf/walkingleaf.sprite", "left", "right")
{
  walk_speed = 60;
  max_drop_height = 16;
}

WalkingLeaf::WalkingLeaf(const Vector& pos, Direction d)
        : WalkingBadguy(pos, d, "images/creatures/walkingleaf/walkingleaf.sprite", "left", "right")
{
  walk_speed = 60;
  max_drop_height = 16;
}

bool
WalkingLeaf::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(object);
  return true;
}

IMPLEMENT_FACTORY(WalkingLeaf, "walkingleaf")
