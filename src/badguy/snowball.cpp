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

#include "badguy/snowball.hpp"

#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "util/gettext.hpp"

SnowBall::SnowBall(const ReaderMapping& reader)
  : WalkingBadguy(reader, "images/creatures/snowball/snowball.sprite", "left", "right")
{
  walk_speed = 80;
}

SnowBall::SnowBall(const Vector& pos, Direction d, std::string script)
  : WalkingBadguy(pos, d, "images/creatures/snowball/snowball.sprite", "left", "right")
{
  walk_speed = 80;
  dead_script = script;
}

bool
SnowBall::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(object);
  return true;
}


ObjectSettings
SnowBall::get_settings() {
  ObjectSettings result(_("Snow ball"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Death script"), &dead_script));
  return result;
}

/* EOF */
