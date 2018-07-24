//  SuperTux
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
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

#include "supertux/player_status.hpp"

#include <algorithm>

#include "audio/sound_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

static const int START_COINS = 100;
static const int MAX_COINS = 9999;

static const int DISPLAYED_COINS_UNSET = -1;

PlayerStatus* player_status = 0;

PlayerStatus::PlayerStatus() :
  /* Do we really want -Weffc++ to bully us into duplicating code from "reset" here? */
  coins(START_COINS),
  bonus(NO_BONUS),
  max_fire_bullets(0),
  max_ice_bullets(0),
  max_air_time(0),
  max_earth_time(0),
  worldmap_sprite("images/worldmap/common/tux.sprite"),
  last_worldmap(),
  displayed_coins(DISPLAYED_COINS_UNSET),
  displayed_coins_frame(0),
  coin_surface(Surface::create("images/engine/hud/coins-0.png"))
{
  reset();

  SoundManager::current()->preload("sounds/coin.wav");
  SoundManager::current()->preload("sounds/lifeup.wav");
}

void PlayerStatus::reset()
{
  coins = START_COINS;
  bonus = NO_BONUS;
  displayed_coins = DISPLAYED_COINS_UNSET;
}

void
PlayerStatus::add_coins(int count, bool play_sound)
{
  coins = std::min(coins + count, MAX_COINS);

  if(!play_sound)
    return;

  static float sound_played_time = 0;
  if(count >= 100)
    SoundManager::current()->play("sounds/lifeup.wav");
  else if (real_time > sound_played_time + 0.010) {
    SoundManager::current()->play("sounds/coin.wav");
    sound_played_time = real_time;
  }
}

void
PlayerStatus::write(Writer& writer)
{
  switch(bonus) {
    case NO_BONUS:
      writer.write("bonus", "none");
      break;
    case GROWUP_BONUS:
      writer.write("bonus", "growup");
      break;
    case FIRE_BONUS:
      writer.write("bonus", "fireflower");
      break;
    case ICE_BONUS:
      writer.write("bonus", "iceflower");
      break;
    case AIR_BONUS:
      writer.write("bonus", "airflower");
      break;
    case EARTH_BONUS:
      writer.write("bonus", "earthflower");
      break;
    default:
      log_warning << "Unknown bonus type." << std::endl;
      writer.write("bonus", "none");
  }
  writer.write("fireflowers", max_fire_bullets);
  writer.write("iceflowers", max_ice_bullets);
  writer.write("airflowers", max_air_time);
  writer.write("earthflowers", max_earth_time);

  writer.write("coins", coins);

  writer.write("worldmap-sprite", worldmap_sprite, false);
  writer.write("last-worldmap", last_worldmap, false);
}

void
PlayerStatus::read(const ReaderMapping& lisp)
{
  reset();

  std::string bonusname;
  if(lisp.get("bonus", bonusname)) {
    if(bonusname == "none") {
      bonus = NO_BONUS;
    } else if(bonusname == "growup") {
      bonus = GROWUP_BONUS;
    } else if(bonusname == "fireflower") {
      bonus = FIRE_BONUS;
    } else if(bonusname == "iceflower") {
      bonus = ICE_BONUS;
    } else if(bonusname == "airflower") {
      bonus = AIR_BONUS;
    } else if(bonusname == "earthflower") {
      bonus = EARTH_BONUS;
    } else {
      log_warning << "Unknown bonus '" << bonusname << "' in savefile" << std::endl;
      bonus = NO_BONUS;
    }
  }
  lisp.get("fireflowers", max_fire_bullets);
  lisp.get("iceflowers", max_ice_bullets);
  lisp.get("airflowers", max_air_time);
  lisp.get("earthflowers", max_earth_time);

  lisp.get("coins", coins);

  lisp.get("worldmap-sprite", worldmap_sprite);
  lisp.get("last-worldmap", last_worldmap);
}

void
PlayerStatus::draw(DrawingContext& context)
{
  int player_id = 0;

  if ((displayed_coins == DISPLAYED_COINS_UNSET) ||
      (std::abs(displayed_coins - coins) > 100)) {
    displayed_coins = coins;
    displayed_coins_frame = 0;
  }
  if (++displayed_coins_frame > 2) {
    displayed_coins_frame = 0;
    if (displayed_coins < coins) displayed_coins++;
    if (displayed_coins > coins) displayed_coins--;
  }
  displayed_coins = std::min(std::max(displayed_coins, 0), MAX_COINS);

  std::stringstream ss;
  ss << displayed_coins;
  std::string coins_text = ss.str();

  context.push_transform();
  context.set_translation(Vector(0, 0));

  if (coin_surface)
  {
    context.color().draw_surface(coin_surface,
                         Vector(context.get_width() - BORDER_X - coin_surface->get_width() - Resources::fixed_font->get_text_width(coins_text),
                                BORDER_Y + 1 + (Resources::fixed_font->get_text_height(coins_text) + 5) * player_id),
                         LAYER_HUD);
  }
  context.color().draw_text(Resources::fixed_font,
                    coins_text,
                    Vector(context.get_width() - BORDER_X - Resources::fixed_font->get_text_width(coins_text),
                           BORDER_Y + (Resources::fixed_font->get_text_height(coins_text) + 5) * player_id),
                    ALIGN_LEFT,
                    LAYER_HUD,
                    PlayerStatus::text_color);

  context.pop_transform();
}

std::string PlayerStatus::get_bonus_prefix() const
{
  switch (bonus) {
  default:
  case NO_BONUS:
    return "small";
  case GROWUP_BONUS:
    return "big";
  case FIRE_BONUS:
    return "fire";
  case ICE_BONUS:
    return "ice";
  case AIR_BONUS:
    return "air";
  case EARTH_BONUS:
    return "earth";
  }
}

/* EOF */
