//  SuperTux - Smart Snowball
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

#include "badguy/smartball.hpp"

#include "audio/sound_manager.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"

SmartBall::SmartBall(const ReaderMapping& reader)
  : WalkingBadguy(reader, "images/creatures/snowball/smart-snowball.sprite", "left", "right")
{
  walk_speed = 80;
  max_drop_height = 16;
}

SmartBall::SmartBall(const Vector& pos, Direction d)
  : WalkingBadguy(pos, d, "images/creatures/snowball/smart-snowball.sprite", "left", "right")
{
  walk_speed = 80;
  max_drop_height = 16;
}

bool
SmartBall::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(object);
  return true;
}


ObjectSettings
SmartBall::get_settings() {
  ObjectSettings result(_("Smart ball"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Death script"), &dead_script));
  return result;
}

/* EOF */
