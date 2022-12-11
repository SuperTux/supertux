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

#include "trigger/door.hpp"

#include "audio/sound_manager.hpp"
#include "math/random.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/fadetoblack.hpp"
#include "supertux/game_session.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "util/reader_mapping.hpp"

Door::Door(const ReaderMapping& mapping) :
  TriggerBase(mapping),
  state(CLOSED),
  target_sector(),
  target_spawnpoint(),
  script(),
  sprite_name("images/objects/door/door.sprite"),
  sprite(),
  lock_sprite(SpriteManager::current()->create("images/objects/door/door_lock.sprite")),
  stay_open_timer(),
  unlocking_timer(),
  lock_warn_timer(),
  m_flip(NO_FLIP),
  m_locked(),
  lock_color(Color::WHITE)
{
  mapping.get("x", m_col.m_bbox.get_left());
  mapping.get("y", m_col.m_bbox.get_top());
  mapping.get("sector", target_sector);
  mapping.get("spawnpoint", target_spawnpoint);
  mapping.get("sprite", sprite_name);
  mapping.get("locked", m_locked);

  state = m_locked ? DoorState::LOCKED : DoorState::CLOSED;

  mapping.get("script", script);

  sprite = SpriteManager::current()->create(sprite_name);
  sprite->set_action("closed");
  m_col.m_bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  std::vector<float> vColor;
  if (mapping.get("lock-color", vColor)) {
    lock_color = Color(vColor);
  }
  else
  {
    lock_color = Color::WHITE;
  }
  lock_sprite->set_color(lock_color);

  SoundManager::current()->preload("sounds/door.wav");
  // TODO: Add proper sounds
  SoundManager::current()->preload("sounds/locked.ogg");
  SoundManager::current()->preload("sounds/turnkey.ogg");
}

Door::Door(int x, int y, const std::string& sector, const std::string& spawnpoint) :
  TriggerBase(),
  state(CLOSED),
  target_sector(sector),
  target_spawnpoint(spawnpoint),
  script(),
  sprite_name("images/objects/door/door.sprite"),
  sprite(SpriteManager::current()->create(sprite_name)),
  lock_sprite(SpriteManager::current()->create("images/objects/door/door_lock.sprite")),
  stay_open_timer(),
  unlocking_timer(),
  lock_warn_timer(),
  m_flip(NO_FLIP),
  lock_color()
{
  state = m_locked ? DoorState::LOCKED : DoorState::CLOSED;
  m_col.m_bbox.set_pos(Vector(static_cast<float>(x), static_cast<float>(y)));

  sprite->set_action("closed");
  m_col.m_bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  lock_sprite->set_color(lock_color);

  SoundManager::current()->preload("sounds/door.wav");
  // TODO: Add proper sounds
  SoundManager::current()->preload("sounds/locked.ogg");
  SoundManager::current()->preload("sounds/turnkey.ogg");
}

ObjectSettings
Door::get_settings()
{
  ObjectSettings result = TriggerBase::get_settings();

  result.add_sprite(_("Sprite"), &sprite_name, "sprite", std::string("images/objects/door/door.sprite"));
  result.add_script(_("Script"), &script, "script");
  result.add_text(_("Sector"), &target_sector, "sector");
  result.add_text(_("Spawn point"), &target_spawnpoint, "spawnpoint");
  result.add_bool(_("Locked?"), &m_locked, "locked");
  result.add_color(_("Lock Color"), &lock_color, "lock-color", Color::WHITE);

  result.reorder({"sector", "lock-color", "locked", "spawnpoint", "name", "x", "y"});

  return result;
}

void
Door::after_editor_set() {
  sprite = SpriteManager::current()->create(sprite_name);
  m_col.m_bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  lock_sprite->set_color(lock_color);
}

Door::~Door()
{
}

void
Door::update(float )
{
  switch (state) {
    case CLOSED:
      break;
    case OPENING:
      // if door has finished opening, start timer and keep door open
      if (sprite->animation_done()) {
        state = OPEN;
        sprite->set_action("open");
        stay_open_timer.start(1.0);
      }
      break;
    case OPEN:
      // if door was open long enough, start closing it
      if (stay_open_timer.check()) {
        state = CLOSING;
        sprite->set_action("closing", 1);
      }
      break;
    case CLOSING:
      // if door has finished closing, keep it shut
      if (sprite->animation_done()) {
        state = CLOSED;
        sprite->set_action("closed");
      }
      break;
    case LOCKED:
      if (lock_warn_timer.check()) {
        lock_warn_timer.stop();
      }
      break;
    case UNLOCKING:
      if (unlocking_timer.check())
      {
        Sector::get().add<SpriteParticle>("images/objects/door/door_lock.sprite",
          "default", get_bbox().get_middle(), ANCHOR_MIDDLE, Vector(0.f, -300.f), Vector(0.f, 1000.f), LAYER_OBJECTS - 2, true, lock_color);
        unlocking_timer.stop();
        state = DoorState::CLOSED;
      }
      break;
  }
}

void
Door::draw(DrawingContext& context)
{
  sprite->draw(context.color(), m_col.m_bbox.p1(), LAYER_BACKGROUNDTILES+1, m_flip);

  if (state == DoorState::LOCKED || state == DoorState::UNLOCKING)
  {
    Vector shake_delta = Vector(static_cast<float>(graphicsRandom.rand(-8, 8)), static_cast<float>(graphicsRandom.rand(-8, 8)));
    float shake_strength = lock_warn_timer.started() ? lock_warn_timer.get_timeleft() : 0.f;
    lock_sprite->draw(context.color(), get_bbox().get_middle() -
      (Vector(lock_sprite->get_width() / 2, lock_sprite->get_height() / 2) + (shake_delta*shake_strength)), LAYER_BACKGROUNDTILES + 1, m_flip);
  }
}

void
Door::event(Player& , EventType type)
{
  switch (state) {
    case CLOSED:
      // if door was activated, start opening it
      if (type == EVENT_ACTIVATE) {
        state = OPENING;
        SoundManager::current()->play("sounds/door.wav", get_pos());
        sprite->set_action("opening", 1);
        ScreenManager::current()->set_screen_fade(std::make_unique<FadeToBlack>(FadeToBlack::FADEOUT, 1.0f));
      }
      break;
    case OPENING:
      break;
    case OPEN:
      break;
    case CLOSING:
      break;
    case LOCKED:
      SoundManager::current()->play("sounds/locked.ogg", get_pos());
      lock_warn_timer.start(0.5f);
      break;
    case UNLOCKING:
      state = CLOSED;
      break;
  }
}

HitResponse
Door::collision(GameObject& other, const CollisionHit& hit_)
{
  switch (state) {
    case CLOSED:
      break;
    case OPENING:
      break;
    case OPEN:
    {
      // if door is open and was touched by a player, teleport the player
      Player* player = dynamic_cast<Player*> (&other);

      if (player) {
        state = CLOSING;
        sprite->set_action("closing", 1);
        if (!script.empty()) {
          Sector::get().run_script(script, "Door");
        }

        if (!target_sector.empty()) {
          GameSession::current()->respawn(target_sector, target_spawnpoint, true);
          ScreenManager::current()->set_screen_fade(std::make_unique<FadeToBlack>(FadeToBlack::FADEIN, 1.0f));
        }
      }
    }
    break;
    case CLOSING:
    case LOCKED:
    case UNLOCKING:
      break;
  }

  return TriggerBase::collision(other, hit_);
}

void
Door::on_flip(float height)
{
  MovingObject::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

void
Door::unlock()
{
  m_locked = false;
  SoundManager::current()->play("sounds/turnkey.ogg", get_pos());
  unlocking_timer.start(1.f);
  state = DoorState::UNLOCKING;
}

/* EOF */
