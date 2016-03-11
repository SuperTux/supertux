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

#include "badguy/bouncing_snowball.hpp"

#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "util/gettext.hpp"

#include <algorithm>

static const float JUMPSPEED = -450;
static const float BSNOWBALL_WALKSPEED = 80;

BouncingSnowball::BouncingSnowball(const ReaderMapping& reader)
  : BadGuy(reader, "images/creatures/bouncing_snowball/bouncing_snowball.sprite")
{
}

BouncingSnowball::BouncingSnowball(const Vector& pos, Direction d)
  : BadGuy(pos, d, "images/creatures/bouncing_snowball/bouncing_snowball.sprite")
{
}

void
BouncingSnowball::initialize()
{
  physic.set_velocity_x(dir == LEFT ? -BSNOWBALL_WALKSPEED : BSNOWBALL_WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
BouncingSnowball::collision_squished(GameObject& object)
{
  sprite->set_action("squished");
  kill_squished(object);
  return true;
}

void
BouncingSnowball::collision_solid(const CollisionHit& hit)
{
  if(sprite->get_action() == "squished")
  {
    return;
  }

  if(hit.bottom) {
    if(get_state() == STATE_ACTIVE) {
      float bounce_speed = -physic.get_velocity_y()*0.8;
      physic.set_velocity_y(std::min(JUMPSPEED, bounce_speed));
    } else {
      physic.set_velocity_y(0);
    }
  } else if(hit.top) {
    physic.set_velocity_y(0);
  }

  // left or right collision
  // The direction must correspond, else we got fake bounces on slopes.
  if((hit.left && dir == LEFT) || (hit.right && dir == RIGHT)) {
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

}

HitResponse
BouncingSnowball::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  collision_solid(hit);
  return CONTINUE;
}


ObjectSettings
BouncingSnowball::get_settings() {
  ObjectSettings result(_("Bouncing snowball"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Death script"), &dead_script));
  return result;
}

/* EOF */
