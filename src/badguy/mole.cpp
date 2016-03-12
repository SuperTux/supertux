//  SuperTux - Mole Badguy
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "audio/sound_manager.hpp"
#include "badguy/mole.hpp"
#include "badguy/mole_rock.hpp"
#include "math/random_generator.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

#include <math.h>

static const float MOLE_WAIT_TIME = 0.2f; /**< time to wait before and after throwing */
static const float THROW_TIME = 4.6f; /**< time to spend throwing */
static const float THROW_INTERVAL = 1; /**< time between two thrown rocks */
static const float THROW_VELOCITY = 400; /**< initial velocity of thrown rocks */

Mole::Mole(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/mole/mole.sprite", LAYER_TILES-1),
  state(PRE_THROWING),
  timer(),
  throw_timer()
{
  physic.enable_gravity(false);
  SoundManager::current()->preload("sounds/fall.wav");
  SoundManager::current()->preload("sounds/squish.wav");
  SoundManager::current()->preload("sounds/dartfire.wav");
}

Mole::Mole(const Vector& pos) :
  BadGuy(pos, "images/creatures/mole/mole.sprite", LAYER_TILES-1),
  state(PRE_THROWING),
  timer(),
  throw_timer()
{
  physic.enable_gravity(false);
  SoundManager::current()->preload("sounds/fall.wav");
  SoundManager::current()->preload("sounds/squish.wav");
  SoundManager::current()->preload("sounds/dartfire.wav");
}

void
Mole::activate()
{
  if (state != DEAD) set_state(PRE_THROWING);
}

void
Mole::kill_fall()
{
  set_state(DEAD);
  SoundManager::current()->play("sounds/fall.wav", get_pos());
  run_dead_script();
}

HitResponse
Mole::collision_badguy(BadGuy& , const CollisionHit& )
{
  return FORCE_MOVE;
}

bool
Mole::collision_squished(GameObject& )
{
  if (frozen)
    return true;

  set_state(DEAD);
  SoundManager::current()->play("sounds/squish.wav", get_pos());
  run_dead_script();
  return true;
}

void
Mole::throw_rock()
{
  float angle = gameRandom.rand(90 - 15, 90 + 15) * (M_PI / 180);
  float vx = cos(angle) * THROW_VELOCITY;
  float vy = -sin(angle) * THROW_VELOCITY;

  SoundManager::current()->play("sounds/dartfire.wav", get_pos());
  Sector::current()->add_object(std::make_shared<MoleRock>(bbox.get_middle(), Vector(vx, vy), this));
}

void
Mole::active_update(float elapsed_time)
{
  BadGuy::active_update(elapsed_time);

  if (frozen)
    return;

  switch (state) {
    case PRE_THROWING:
      if (timer.check()) {
        set_state(THROWING);
      }
      break;
    case THROWING:
      if (throw_timer.check()) {
        throw_rock();
        throw_timer.start(THROW_INTERVAL);
      }
      if (timer.check()) {
        set_state(POST_THROWING);
      }
      break;
    case POST_THROWING:
      if (timer.check()) {
        set_state(PEEKING);
      }
      break;
    case PEEKING:
      if (sprite->animation_done()) {
        set_state(PRE_THROWING);
      }
      break;
    case BURNING:
      if (sprite->animation_done()) {
        set_state(DEAD);
      }
      break;
    case DEAD:
      break;
  }

}

bool
Mole::is_freezable() const
{
  return true;
}

void
Mole::set_state(MoleState new_state)
{
  if (frozen)
    return;

  switch (new_state) {
    case PRE_THROWING:
      sprite->set_action("idle");
      set_colgroup_active(COLGROUP_DISABLED);
      timer.start(MOLE_WAIT_TIME);
      break;
    case THROWING:
      sprite->set_action("idle");
      set_colgroup_active(COLGROUP_DISABLED);
      timer.start(THROW_TIME);
      throw_timer.start(THROW_INTERVAL);
      break;
    case POST_THROWING:
      sprite->set_action("idle");
      set_colgroup_active(COLGROUP_DISABLED);
      timer.start(MOLE_WAIT_TIME);
      break;
    case PEEKING:
      sprite->set_action("peeking", 1);
      set_colgroup_active(COLGROUP_STATIC);
      break;
    case DEAD:
      sprite->set_action("idle");
      set_colgroup_active(COLGROUP_DISABLED);
      break;
    case BURNING:
      sprite->set_action("burning", 1);
      set_colgroup_active(COLGROUP_DISABLED);
      break;
  }

  state = new_state;
}

void
Mole::ignite() {
  set_state(BURNING);
}

/* EOF */
