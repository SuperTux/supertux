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

#include <sstream>

#include "audio/sound_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/game_session.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

static const int START_COINS = 100;
static const int MAX_COINS = 9999;

PlayerStatus::PlayerStatus() :
  coins(START_COINS),
  bonus(NO_BONUS),
  max_fire_bullets(0),
  max_ice_bullets(0),
  max_air_time(0),
  max_earth_time(0),
  worldmap_sprite("images/worldmap/common/tux.sprite"),
  last_worldmap(),
  last_worldmap_title()
{
  reset();

  // FIXME: Move sound handling into PlayerStatusHUD
  if (SoundManager::current()) {
    SoundManager::current()->preload("sounds/coin.wav");
    SoundManager::current()->preload("sounds/lifeup.wav");
  }
}

void
PlayerStatus::take_checkpoint_coins()
{
  int subtract_value = std::max(coins / 10, 25);
  if (coins - subtract_value >= 0)
    coins -= subtract_value;
  else
    coins = 0;
}

void PlayerStatus::reset()
{
  coins = START_COINS;
  bonus = NO_BONUS;
}

int
PlayerStatus::get_max_coins() const
{
  return MAX_COINS;
}

bool
PlayerStatus::can_reach_checkpoint() const
{
  return !GameSession::current()->get_reset_point_sectorname().empty();
}

void
PlayerStatus::add_coins(int count, bool play_sound)
{
  coins = std::min(coins + count, MAX_COINS);

  if (!play_sound)
    return;

  static float sound_played_time = 0;
  if (count >= 100)
    SoundManager::current()->play("sounds/lifeup.wav");
  else if (g_real_time > sound_played_time + 0.010f) {
    SoundManager::current()->play("sounds/coin.wav");
    sound_played_time = g_real_time;
  }
}

void
PlayerStatus::write(Writer& writer)
{
  switch (bonus) {
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
  writer.write("last-worldmap-title", last_worldmap_title, false);
}

void
PlayerStatus::read(const ReaderMapping& mapping)
{
  reset();

  std::string bonusname;
  if (mapping.get("bonus", bonusname)) {
    if (bonusname == "none") {
      bonus = NO_BONUS;
    } else if (bonusname == "growup") {
      bonus = GROWUP_BONUS;
    } else if (bonusname == "fireflower") {
      bonus = FIRE_BONUS;
    } else if (bonusname == "iceflower") {
      bonus = ICE_BONUS;
    } else if (bonusname == "airflower") {
      bonus = AIR_BONUS;
    } else if (bonusname == "earthflower") {
      bonus = EARTH_BONUS;
    } else {
      log_warning << "Unknown bonus '" << bonusname << "' in savefile" << std::endl;
      bonus = NO_BONUS;
    }
  }
  mapping.get("fireflowers", max_fire_bullets);
  mapping.get("iceflowers", max_ice_bullets);
  mapping.get("airflowers", max_air_time);
  mapping.get("earthflowers", max_earth_time);

  mapping.get("coins", coins);

  mapping.get("worldmap-sprite", worldmap_sprite);
  mapping.get("last-worldmap", last_worldmap);
  if (!mapping.get("last-worldmap-title", last_worldmap_title))
    last_worldmap_title = "";
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
