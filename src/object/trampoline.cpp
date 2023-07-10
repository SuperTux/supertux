//  SuperTux - Trampoline
//  Copyright (C) 2006 Wolfgang Becker <uafr@gmx.de>
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

#include "object/trampoline.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/walking_badguy.hpp"
#include "control/controller.hpp"
#include "object/player.hpp"
#include "object/coin.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "util/reader_mapping.hpp"

/* Trampoline will accelerate Tux to to VY_INITIAL, if
 * he jumps on it to VY_MIN. */
namespace {
const std::string TRAMPOLINE_SOUND = "sounds/trampoline.wav";
const float VY_MIN = -900; //negative, upwards
const float VY_INITIAL = -500;
}

Trampoline::Trampoline(const ReaderMapping& mapping) :
  Rock(mapping, "images/objects/trampoline/trampoline.sprite"),
  portable(true)
{
  SoundManager::current()->preload(TRAMPOLINE_SOUND);

  //Check if this trampoline is not portable
  if (mapping.get("portable", portable)) {
    if (!portable && !mapping.get("sprite", m_sprite_name)) {
      //we need another sprite
      m_sprite_name = "images/objects/trampoline/trampoline_fix.sprite";
      m_default_sprite_name = m_sprite_name;
      m_sprite = SpriteManager::current()->create(m_sprite_name);
      m_sprite->set_action("normal");
    }
  }
}

Trampoline::Trampoline(const Vector& pos, bool port) :
  Rock(pos, "images/objects/trampoline/trampoline.sprite"),
  portable(port)
{
  SoundManager::current()->preload(TRAMPOLINE_SOUND);
  if (!port) {
    m_sprite_name = "images/objects/trampoline/trampoline_fix.sprite";
    m_sprite = SpriteManager::current()->create(m_sprite_name);
    m_sprite->set_action("normal");
  }
}

void
Trampoline::update(float dt_sec)
{
  if (m_sprite->animation_done()) {
    m_sprite->set_action("normal");
  }

  Rock::update(dt_sec);
}

HitResponse
Trampoline::collision(GameObject& other, const CollisionHit& hit)
{
  auto heavy_coin = dynamic_cast<HeavyCoin*> (&other);
  if (heavy_coin) {
    return ABORT_MOVE;
  }
  //Tramponine has to be on ground to work.
  if (on_ground) {
    auto player = dynamic_cast<Player*> (&other);
    //Trampoline works for player
    if (player) {
      player->override_velocity();
      if (player->m_does_buttjump)
        player->m_does_buttjump = false;
      float vy = player->get_physic().get_velocity_y();
      //player is falling down on trampoline
      if (hit.top && vy >= 0) {
        if (!(player->get_status().bonus[player->get_id()] == AIR_BONUS))
        {
          if (player->get_controller().hold(Control::JUMP))
            vy = VY_MIN;
          else if (player->get_controller().hold(Control::DOWN))
            vy = VY_MIN + 100;
          else
            vy = VY_INITIAL;
        }
        else
        {
          if (player->get_controller().hold(Control::JUMP))
            vy = VY_MIN - 80;
          else if (player->get_controller().hold(Control::DOWN))
            vy = VY_MIN - 70;
          else
            vy = VY_INITIAL - 40;
        }
        player->get_physic().set_velocity_y(vy);
        bounce();
        return FORCE_MOVE;
      }
    }
    auto walking_badguy = dynamic_cast<WalkingBadguy*> (&other);
    //Trampoline also works for WalkingBadguy
    if (walking_badguy) {
      float vy = walking_badguy->get_velocity_y();
      //walking_badguy is falling down on trampoline
      if (hit.top && vy >= 0) {
        vy = VY_INITIAL;
        walking_badguy->set_velocity_y(vy);
        SoundManager::current()->play(TRAMPOLINE_SOUND, get_pos());
        m_sprite->set_action("swinging", 1);
        return FORCE_MOVE;
      }
    }
  }

  return Rock::collision(other, hit);
}

void
Trampoline::grab(MovingObject& object, const Vector& pos, Direction dir) {
  m_sprite->set_animation_loops(0);
  Rock::grab(object, pos, dir);
}

bool
Trampoline::is_portable() const
{
  return Rock::is_portable() && portable;
}

ObjectSettings
Trampoline::get_settings()
{
  ObjectSettings result = Rock::get_settings();

  result.add_bool(_("Portable"), &portable, "portable", true);

  result.reorder({"portable", "sprite", "x", "y"});

  return result;
}

void
Trampoline::bounce()
{
  SoundManager::current()->play(TRAMPOLINE_SOUND, get_pos());
  m_sprite->set_action("swinging", 1);
}

/* EOF */
