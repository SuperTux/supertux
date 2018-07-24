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
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/fadein.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/game_session.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

Door::Door(const ReaderMapping& reader) :
  state(CLOSED),
  target_sector(),
  target_spawnpoint(),
  script(),
  sprite(SpriteManager::current()->create("images/objects/door/door.sprite")),
  stay_open_timer()
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  reader.get("sector", target_sector);
  reader.get("spawnpoint", target_spawnpoint);

  reader.get("script", script);

  sprite->set_action("closed");
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  SoundManager::current()->preload("sounds/door.wav");
}

Door::Door(int x, int y, const std::string& sector, const std::string& spawnpoint) :
  state(CLOSED),
  target_sector(sector),
  target_spawnpoint(spawnpoint),
  script(),
  sprite(SpriteManager::current()->create("images/objects/door/door.sprite")),
  stay_open_timer()
{
  bbox.set_pos(Vector(x, y));

  sprite->set_action("closed");
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  SoundManager::current()->preload("sounds/door.wav");
}

ObjectSettings
Door::get_settings() {
  ObjectSettings result(_("Door"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Sector"), &target_sector, "sector"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Spawn point"), &target_spawnpoint, "spawnpoint"));
  return result;
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
  sprite->draw(context.color(), bbox.p1, LAYER_BACKGROUNDTILES+1);
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
        bool invincible = player->is_invincible();
        int invincibilityperiod = player->invincible_timer.get_timeleft();
        state = CLOSING;
        sprite->set_action("closing", 1);
        if(!script.empty()) {
          Sector::current()->run_script(script, "Door");
        }

        if(!target_sector.empty()) {
          GameSession::current()->respawn(target_sector, target_spawnpoint,
                                          invincible, invincibilityperiod);
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
