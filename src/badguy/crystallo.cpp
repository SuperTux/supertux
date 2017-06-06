//  SuperTux - Crystallo
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
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

#include "badguy/crystallo.hpp"

#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "util/reader_mapping.hpp"
#include "object/anchor_point.hpp"

Crystallo::Crystallo(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/crystallo/crystallo.sprite", "left", "right"),
  radius()
{
  walk_speed = 80;
  max_drop_height = 16;

  reader.get("radius", radius, 100);
}

Crystallo::Crystallo(const Vector& pos, Direction d) :
  WalkingBadguy(pos, d, "images/creatures/crystallo/crystallo.sprite", "left", "right"),
  radius(100)
{
  walk_speed = 80;
  max_drop_height = 16;
}

ObjectSettings
Crystallo::get_settings() {
  ObjectSettings result = WalkingBadguy::get_settings();
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Radius"), &radius,
                                         "radius"));
  return result;
}

void
Crystallo::active_update(float elapsed_time)
{
  if(get_pos().x > (start_position.x + radius)){
    if(dir != LEFT){
      turn_around();
    }
  }
  if( get_pos().x < (start_position.x - radius)){
    if(dir != RIGHT){
      turn_around();
    }
  }
  BadGuy::active_update(elapsed_time);
}

bool
Crystallo::collision_squished(GameObject& object)
{
  this->set_action(dir == LEFT ? "shattered-left" : "shattered-right", /* loops = */ -1, ANCHOR_BOTTOM);
  kill_squished(object);
  return true;
}

bool
Crystallo::is_flammable() const
{
  return false;
}

/* EOF */
