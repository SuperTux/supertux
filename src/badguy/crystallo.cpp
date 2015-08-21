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
#include "util/reader.hpp"
#include "object/anchor_point.hpp"
#include "util/gettext.hpp"

Crystallo::Crystallo(const Reader& reader) :
  WalkingBadguy(reader, "images/creatures/crystallo/crystallo.sprite", "left", "right"),
  radius()
{
  walk_speed = 80;
  max_drop_height = 16;
  radius = 100;
  reader.get("radius", radius);
}

Crystallo::Crystallo(const Vector& pos, Direction d) :
  WalkingBadguy(pos, d, "images/creatures/crystallo/crystallo.sprite", "left", "right"),
  radius()
{
  walk_speed = 80;
  max_drop_height = 16;
  radius = 100;
}

void
Crystallo::save(lisp::Writer& writer){
  BadGuy::save(writer);
  writer.write("radius",radius);
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


ObjectSettings
Crystallo::get_settings() {
  ObjectSettings result(_("Crystallo"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Death script"), &dead_script));
  return result;
}
/* EOF */
