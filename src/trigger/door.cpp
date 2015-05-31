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

#include <sstream>

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/fadein.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/game_session.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader.hpp"

Door::Door(const Reader& reader) :
  state(CLOSED),
  target_sector(),
  target_spawnpoint(),
  script(),
  sprite(),
  stay_open_timer()
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  reader.get("sector", target_sector);
  reader.get("spawnpoint", target_spawnpoint);

  reader.get("script", script);

  sprite = SpriteManager::current()->create("images/objects/door/door.sprite");
  sprite->set_action("closed");
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  SoundManager::current()->preload("sounds/door.wav");
}

Door::Door(int x, int y, std::string sector, std::string spawnpoint) :
  state(CLOSED),
  target_sector(),
  target_spawnpoint(),
  script(),
  sprite(),
  stay_open_timer()
{
  bbox.set_pos(Vector(x, y));
  target_sector = sector;
  target_spawnpoint = spawnpoint;

  sprite = SpriteManager::current()->create("images/objects/door/door.sprite");
  sprite->set_action("closed");
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  SoundManager::current()->preload("sounds/door.wav");
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
      if(sprite->animation_done()) {
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
      if(sprite->animation_done()) {
        state = CLOSED;
        sprite->set_action("closed");
      }
      break;
  }
}

void
Door::draw(DrawingContext& context)
{
  sprite->draw(context, bbox.p1, LAYER_BACKGROUNDTILES+1);
}

void
Door::event(Player& , EventType type)
{
  switch (state) {
    case CLOSED:
      // if door was activated, start opening it
      if (type == EVENT_ACTIVATE) {
        state = OPENING;
        SoundManager::current()->play("sounds/door.wav");
        sprite->set_action("opening", 1);
        ScreenManager::current()->set_screen_fade(std::unique_ptr<ScreenFade>(new FadeOut(1)));
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
        state = CLOSING;
        sprite->set_action("closing", 1);
        if(!script.empty()) {
          std::istringstream stream(script);
          Sector::current()->run_script(stream, "Door");
        }

        if(!target_sector.empty()) {
          GameSession::current()->respawn(target_sector, target_spawnpoint);
          ScreenManager::current()->set_screen_fade(std::unique_ptr<ScreenFade>(new FadeIn(1)));
        }
      }
    }
    break;
    case CLOSING:
      break;
  }

  return TriggerBase::collision(other, hit_);
}

/* EOF */
