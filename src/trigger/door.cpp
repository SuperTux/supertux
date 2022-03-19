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
#include "object/player.hpp"
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
  stay_open_timer(),
  m_flip(NO_FLIP)
{
  mapping.get("x", m_col.m_bbox.get_left());
  mapping.get("y", m_col.m_bbox.get_top());
  mapping.get("sector", target_sector);
  mapping.get("spawnpoint", target_spawnpoint);
  mapping.get("sprite", sprite_name);

  mapping.get("script", script);

  sprite = SpriteManager::current()->create(sprite_name);
  sprite->set_action("closed");
  m_col.m_bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  SoundManager::current()->preload("sounds/door.wav");
}

Door::Door(int x, int y, const std::string& sector, const std::string& spawnpoint) :
  TriggerBase(),
  state(CLOSED),
  target_sector(sector),
  target_spawnpoint(spawnpoint),
  script(),
  sprite_name("images/objects/door/door.sprite"),
  sprite(SpriteManager::current()->create(sprite_name)),
  stay_open_timer(),
  m_flip(NO_FLIP)
{
  m_col.m_bbox.set_pos(Vector(static_cast<float>(x), static_cast<float>(y)));

  sprite->set_action("closed");
  m_col.m_bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  SoundManager::current()->preload("sounds/door.wav");
}

ObjectSettings
Door::get_settings()
{
  ObjectSettings result = TriggerBase::get_settings();

  result.add_sprite(_("Sprite"), &sprite_name, "sprite", std::string("images/objects/door/door.sprite"));
  result.add_script(_("Script"), &script, "script");
  result.add_text(_("Sector"), &target_sector, "sector");
  result.add_text(_("Spawn point"), &target_spawnpoint, "spawnpoint");

  result.reorder({"sector", "spawnpoint", "name", "x", "y"});

  return result;
}

void
Door::after_editor_set() {
  sprite = SpriteManager::current()->create(sprite_name);
  m_col.m_bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
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
  }
}

void
Door::draw(DrawingContext& context)
{
  sprite->draw(context.color(), m_col.m_bbox.p1(), LAYER_BACKGROUNDTILES+1, m_flip);
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
        bool invincible = player->is_invincible();
        int invincibilityperiod = static_cast<int>(player->m_invincible_timer.get_timeleft());
        state = CLOSING;
        sprite->set_action("closing", 1);
        if (!script.empty()) {
          Sector::get().run_script(script, "Door");
        }

        if (!target_sector.empty()) {
          GameSession::current()->respawn(target_sector, target_spawnpoint,
                                          invincible, invincibilityperiod);
          ScreenManager::current()->set_screen_fade(std::make_unique<FadeToBlack>(FadeToBlack::FADEIN, 1.0f));
        }
      }
    }
    break;
    case CLOSING:
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

/* EOF */
