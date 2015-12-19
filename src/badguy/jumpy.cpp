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

#include "badguy/jumpy.hpp"

#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "util/gettext.hpp"

#include <algorithm>

static const float JUMPYSPEED=-600;
static const float JUMPY_MID_TOLERANCE=4;
static const float JUMPY_LOW_TOLERANCE=2;

Jumpy::Jumpy(const Reader& reader) :
  BadGuy(reader, "images/creatures/snowjumpy/snowjumpy.sprite"),
  pos_groundhit(),
  groundhit_pos_set(false)
{
  // TODO create a nice sound for this...
  //SoundManager::current()->preload("sounds/skid.wav");
}

void
Jumpy::collision_solid(const CollisionHit& chit)
{
  hit(chit);
}

HitResponse
Jumpy::collision_badguy(BadGuy& , const CollisionHit& chit)
{
  return hit(chit);
}

HitResponse
Jumpy::hit(const CollisionHit& chit)
{
  if(chit.bottom) {
    if (!groundhit_pos_set)
    {
      pos_groundhit = get_pos();
      groundhit_pos_set = true;
    }

    physic.set_velocity_y((frozen || get_state() != STATE_ACTIVE) ? 0 : JUMPYSPEED);
    // TODO create a nice sound for this...
    //SoundManager::current()->play("sounds/skid.wav");
    update_on_ground_flag(chit);
  } else if(chit.top) {
    physic.set_velocity_y(0);
  }

  return CONTINUE;
}

void
Jumpy::active_update(float elapsed_time)
{
  BadGuy::active_update(elapsed_time);

  if(frozen)
    return;

  Player* player = get_nearest_player();
  if (player)
  {
    dir = (player->get_pos().x > get_pos().x) ? RIGHT : LEFT;
  }

  if (!groundhit_pos_set)
  {
    sprite->set_action(dir == LEFT ? "left-middle" : "right-middle");
    return;
  }

  if ( get_pos().y < (pos_groundhit.y - JUMPY_MID_TOLERANCE ) )
    sprite->set_action(dir == LEFT ? "left-up" : "right-up");
  else if ( get_pos().y >= (pos_groundhit.y - JUMPY_MID_TOLERANCE) &&
            get_pos().y < (pos_groundhit.y - JUMPY_LOW_TOLERANCE) )
    sprite->set_action(dir == LEFT ? "left-middle" : "right-middle");
  else
    sprite->set_action(dir == LEFT ? "left-down" : "right-down");
}

void
Jumpy::freeze()
{
  BadGuy::freeze();
  physic.set_velocity_y(std::max(0.0f, physic.get_velocity_y()));
}

bool
Jumpy::is_freezable() const
{
  return true;
}

ObjectSettings
Jumpy::get_settings() {
  ObjectSettings result(_("Jumpy"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Death script"), &dead_script));
  return result;
}

bool
Jumpy::is_flammable() const
{
  return true;
}

/* EOF */
