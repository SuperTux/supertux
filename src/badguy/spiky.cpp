//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "spiky.hpp"

#include "lisp/writer.hpp"
#include "object_factory.hpp"
#include "sprite/sprite.hpp"

Spiky::Spiky(const lisp::Lisp& reader)
        : WalkingBadguy(reader, "images/creatures/spiky/spiky.sprite", "left", "right")
{
  walk_speed = 80;
  max_drop_height = 600;
}

void
Spiky::write(lisp::Writer& writer)
{
  writer.start_list("spiky");
  WalkingBadguy::write(writer);
  writer.end_list("spiky");
}

void
Spiky::freeze()
{
  WalkingBadguy::freeze();
  sprite->set_action(dir == LEFT ? "iced-left" : "iced-right");
}

bool
Spiky::is_freezable() const
{
  return true;
}

IMPLEMENT_FACTORY(Spiky, "spiky")
