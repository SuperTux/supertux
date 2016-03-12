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

#include "badguy/sspiky.hpp"

#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "util/gettext.hpp"

SSpiky::SSpiky(const ReaderMapping& reader)
  : WalkingBadguy(reader, "images/creatures/spiky/sleepingspiky.sprite", "left", "right"), state(SSPIKY_SLEEPING)
{
  walk_speed = 80;
  max_drop_height = 600;
}

void
SSpiky::initialize()
{
  state = SSPIKY_SLEEPING;
  physic.set_velocity_x(0);
  sprite->set_action(dir == LEFT ? "sleeping-left" : "sleeping-right");
}

void
SSpiky::collision_solid(const CollisionHit& hit)
{
  if(state != SSPIKY_WALKING) {
    BadGuy::collision_solid(hit);
    return;
  }
  WalkingBadguy::collision_solid(hit);
}

HitResponse
SSpiky::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  if(state != SSPIKY_WALKING) {
    return BadGuy::collision_badguy(badguy, hit);
  }
  return WalkingBadguy::collision_badguy(badguy, hit);
}

void
SSpiky::active_update(float elapsed_time) {

  if(state == SSPIKY_WALKING) {
    WalkingBadguy::active_update(elapsed_time);
    return;
  }

  if(state == SSPIKY_SLEEPING) {

    Player* player = get_nearest_player();
    if (player) {
      Rectf pb = player->get_bbox();

      bool inReach_left = (pb.p2.x >= bbox.p2.x-((dir == LEFT) ? 256 : 0));
      bool inReach_right = (pb.p1.x <= bbox.p1.x+((dir == RIGHT) ? 256 : 0));
      bool inReach_top = (pb.p2.y >= bbox.p1.y);
      bool inReach_bottom = (pb.p1.y <= bbox.p2.y);

      if (inReach_left && inReach_right && inReach_top && inReach_bottom) {
        // wake up
        sprite->set_action(dir == LEFT ? "waking-left" : "waking-right", 1);
        state = SSPIKY_WAKING;
      }
    }

    BadGuy::active_update(elapsed_time);
  }

  if(state == SSPIKY_WAKING) {
    if(sprite->animation_done()) {
      // start walking
      state = SSPIKY_WALKING;
      WalkingBadguy::initialize();
    }

    BadGuy::active_update(elapsed_time);
  }
}

void
SSpiky::freeze()
{
  WalkingBadguy::freeze();
  state = SSPIKY_WALKING; // if we get hit while sleeping, wake up :)
}

bool
SSpiky::is_freezable() const
{
  return true;
}

bool
SSpiky::is_flammable() const
{
  return true;
}

ObjectSettings
SSpiky::get_settings() {
  ObjectSettings result(_("Sleeping spiky"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Death script"), &dead_script));
  return result;
}

/* EOF */
