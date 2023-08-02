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

PlayerStatus::PlayerStatus(int num_players) :
  m_num_players(num_players),
  coins(START_COINS),
  bonus(num_players),
  max_fire_bullets(num_players),
  max_ice_bullets(num_players),
  max_air_time(num_players),
  max_earth_time(num_players),
  worldmap_sprite("images/worldmap/common/tux.sprite"),
  last_worldmap()
{
  reset(num_players);

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

void PlayerStatus::reset(int num_players)
{
  coins = START_COINS;

  // Keep in sync with a section in read()
  bonus.clear();
  bonus.resize(num_players, NO_BONUS);
  max_fire_bullets.clear();
  max_fire_bullets.resize(num_players, 0);
  max_ice_bullets.clear();
  max_ice_bullets.resize(num_players, 0);
  max_air_time.clear();
  max_air_time.resize(num_players, 0);
  max_earth_time.clear();
  max_earth_time.resize(num_players, 0);

  m_num_players = num_players;
}

int
PlayerStatus::get_max_coins() const
{
  return MAX_COINS;
}

bool
PlayerStatus::can_reach_checkpoint() const
{
  return GameSession::current()->get_active_checkpoint_spawnpoint();
}

bool
PlayerStatus::respawns_at_checkpoint() const
{
  return GameSession::current()->get_last_spawnpoint().is_checkpoint ||
         GameSession::current()->reset_checkpoint_button;
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
  writer.write("num_players", m_num_players);

  for (int i = 0; i < m_num_players; i++)
  {
    if (i != 0)
    {
      writer.start_list("tux" + std::to_string(i + 1));
    }

    switch (bonus[i]) {
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

    writer.write("fireflowers", max_fire_bullets[i]);
    writer.write("iceflowers", max_ice_bullets[i]);
    writer.write("airflowers", max_air_time[i]);
    writer.write("earthflowers", max_earth_time[i]);

    if (i != 0)
    {
      writer.end_list("tux" + std::to_string(i + 1));
    }
  }

  writer.write("coins", coins);

  writer.write("worldmap-sprite", worldmap_sprite, false);
  writer.write("last-worldmap", last_worldmap, false);
}

void
PlayerStatus::read(const ReaderMapping& mapping)
{
  int num_players_in_file = 1;
  mapping.get("num_players", num_players_in_file);

  reset(std::max(m_num_players, num_players_in_file));

  auto iter = mapping.get_iter();

  while (iter.next())
  {
    try
    {
      if (iter.get_key().size() > 3 && iter.get_key().substr(0, 3) == "tux")
      {
        int id = std::stoi(iter.get_key().substr(3)) - 1;

        if (id >= m_num_players)
        {
          log_warning << "ID larger than amount of players when reading player state: " << id << std::endl;

          // Keep this in sync with reset()
          if (bonus.size() < static_cast<size_t>(id))
            bonus.resize(id, NO_BONUS);

          if (max_fire_bullets.size() < static_cast<size_t>(id))
            max_fire_bullets.resize(id, 0);

          if (max_ice_bullets.size() < static_cast<size_t>(id))
            max_ice_bullets.resize(id, 0);

          if (max_air_time.size() < static_cast<size_t>(id))
            max_air_time.resize(id, 0);

          if (max_earth_time.size() < static_cast<size_t>(id))
            max_earth_time.resize(id, 0);
        }
        else if (id == 0)
        {
          log_warning << "Refusing to parse player 1 when reading player state,"
                         "please don't put player 1 data in a (tux1 ...)"
                         "wrapper for retrocompatibiility" << std::endl;
        }

        auto map = iter.as_mapping();
        parse_bonus_mapping(map, id);
      }
    }
    catch (const std::exception& e)
    {
      log_warning << "Couldn't parse player from player status save: " << e.what() << std::endl;
    }
  }

  parse_bonus_mapping(mapping, 0);

  mapping.get("coins", coins);

  mapping.get("worldmap-sprite", worldmap_sprite);
  mapping.get("last-worldmap", last_worldmap);
}

void
PlayerStatus::parse_bonus_mapping(const ReaderMapping& map, int id)
{
  std::string bonusname;
  if (map.get("bonus", bonusname)) {
    if (bonusname == "none") {
      bonus[id] = NO_BONUS;
    } else if (bonusname == "growup") {
      bonus[id] = GROWUP_BONUS;
    } else if (bonusname == "fireflower") {
      bonus[id] = FIRE_BONUS;
    } else if (bonusname == "iceflower") {
      bonus[id] = ICE_BONUS;
    } else if (bonusname == "airflower") {
      bonus[id] = AIR_BONUS;
    } else if (bonusname == "earthflower") {
      bonus[id] = EARTH_BONUS;
    } else {
      log_warning << "Unknown bonus '" << bonusname << "' in savefile for player " << (id + 1) << std::endl;
      bonus[id] = NO_BONUS;
    }
  }
  map.get("fireflowers", max_fire_bullets[id]);
  map.get("iceflowers", max_ice_bullets[id]);
  map.get("airflowers", max_air_time[id]);
  map.get("earthflowers", max_earth_time[id]);
}

std::string
PlayerStatus::get_bonus_prefix(int player_id) const
{
  switch (bonus[player_id]) {
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

void
PlayerStatus::add_player()
{
  m_num_players++;

  bonus.resize(m_num_players, NO_BONUS);
  max_fire_bullets.resize(m_num_players, 0);
  max_ice_bullets.resize(m_num_players, 0);
  max_air_time.resize(m_num_players, 0);
  max_earth_time.resize(m_num_players, 0);
}

void
PlayerStatus::remove_player(int player_id)
{
  m_num_players--;

  for (int i = player_id; i < m_num_players; i++)
  {
    bonus[i] = bonus[i + 1];
    max_fire_bullets[i] = max_fire_bullets[i + 1];
    max_ice_bullets[i] = max_ice_bullets[i + 1];
    max_air_time[i] = max_air_time[i + 1];
    max_earth_time[i] = max_earth_time[i + 1];
  }

  bonus.resize(m_num_players, NO_BONUS);
  max_fire_bullets.resize(m_num_players, 0);
  max_ice_bullets.resize(m_num_players, 0);
  max_air_time.resize(m_num_players, 0);
  max_earth_time.resize(m_num_players, 0);
}

/* EOF */
