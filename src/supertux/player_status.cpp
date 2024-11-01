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
#include "supertux/player_status.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

static const int START_COINS = 100;
static const int MAX_COINS = 9999;

PlayerStatus::Status::Status(PlayerStatus& status) :
  general_status(status),
  bonus(NO_BONUS),
  max_fire_bullets(),
  max_ice_bullets(),
  max_air_time(),
  max_earth_time(),
  m_saved_state()
{
}

PlayerStatus::Status::Status(const PlayerStatus::Status& other) :
  general_status(other.general_status),
  bonus(other.bonus),
  max_fire_bullets(other.max_fire_bullets),
  max_ice_bullets(other.max_ice_bullets),
  max_air_time(other.max_air_time),
  max_earth_time(other.max_earth_time),
  m_saved_state()
{
}

void
PlayerStatus::Status::save_state()
{
  m_saved_state.reset(new Status(*this));
}

void
PlayerStatus::Status::restore_state()
{
  if (!m_saved_state) return;

  bonus = m_saved_state->bonus;
  max_fire_bullets = m_saved_state->max_fire_bullets;
  max_ice_bullets = m_saved_state->max_ice_bullets;
  max_air_time = m_saved_state->max_air_time;
  max_earth_time = m_saved_state->max_earth_time;

  m_saved_state.reset();
}

void
PlayerStatus::Status::parse(const ReaderMapping& reader)
{
  std::string bonusname;
  if (reader.get("bonus", bonusname)) {
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
      log_warning << "Unknown bonus '" << bonusname << "' in savefile for player" << std::endl;
      bonus = NO_BONUS;
    }
  }
  reader.get("fireflowers", max_fire_bullets);
  reader.get("iceflowers", max_ice_bullets);
  reader.get("airflowers", max_air_time);
  reader.get("earthflowers", max_earth_time);
}

void
PlayerStatus::Status::write(Writer& writer) const
{
  switch (bonus)
  {
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
      break;
  }

  writer.write("fireflowers", max_fire_bullets);
  writer.write("iceflowers", max_ice_bullets);
  writer.write("airflowers", max_air_time);
  writer.write("earthflowers", max_earth_time);
}

std::string
PlayerStatus::Status::get_bonus_prefix() const
{
  switch (bonus)
  {
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


PlayerStatus::PlayerStatus() :
  m_local_players(),
  m_remote_players(),
  m_saved_coins(START_COINS),
  coins(START_COINS),
  worldmap_sprite("images/worldmap/common/tux.sprite"),
  last_worldmap(),
  title_level()
{
  // FIXME: Move sound handling into PlayerStatusHUD
  if (SoundManager::current()) {
    SoundManager::current()->preload("sounds/coin.wav");
    SoundManager::current()->preload("sounds/lifeup.wav");
  }
}

void
PlayerStatus::save_state()
{
  m_saved_coins = coins;

  for (auto& player : m_local_players)
    player->save_state();

  for (auto& [_, players] : m_remote_players)
  {
    for (auto& player : players)
      player->save_state();
  }
}

void
PlayerStatus::restore_state()
{
  coins = m_saved_coins;

  for (auto& player : m_local_players)
    player->restore_state();

  for (auto& [_, players] : m_remote_players)
  {
    for (auto& player : players)
      player->restore_state();
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

void
PlayerStatus::reset()
{
  m_local_players.clear();
  m_local_players.push_back(std::make_unique<Status>(*this)); // Always have status for player 1.

  m_remote_players.clear();

  m_saved_coins = START_COINS;
  coins = START_COINS;

  worldmap_sprite.clear();
  last_worldmap.clear();
  title_level.clear();
}

void
PlayerStatus::expand(std::vector<std::unique_ptr<Status>>& vec, int n)
{
  while (static_cast<int>(vec.size()) < n)
    vec.push_back(std::make_unique<Status>(*this));
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

std::string
PlayerStatus::write(bool include_world_data) const
{
  std::ostringstream stream;
  Writer writer(stream);

  writer.start_list("supertux-player-status");
  write(writer, include_world_data);
  writer.end_list("supertux-player-status");

  return stream.str();
}

void
PlayerStatus::read(const std::string& data, const std::string& self_username, const std::string& remote_username)
{
  auto doc = ReaderDocument::from_string(data, remote_username.empty() ? "player-status" : "remote-player-status");
  auto root = doc.get_root();
  if (root.get_name() != "supertux-player-status")
    throw std::runtime_error("PlayerStatus data is not in \"supertux-player-status\" format!");

  read(root.get_mapping(), self_username, remote_username);
}

void
PlayerStatus::write(Writer& writer, bool include_world_data) const
{
  for (int i = 0; i < get_num_local_players(); i++)
  {
    if (i != 0)
      writer.start_list("tux" + std::to_string(i + 1));

    m_local_players[i]->write(writer);

    if (i != 0)
      writer.end_list("tux" + std::to_string(i + 1));
  }
  for (const auto& [username, players] : m_remote_players)
  {
    for (int i = 0; i < static_cast<int>(players.size()); i++)
    {
      writer.start_list("tux_remote");

      writer.write("username", username);
      writer.write("id", i + 1);
      players[i]->write(writer);

      writer.end_list("tux_remote");
    }
  }

  writer.write("coins", coins);

  if (include_world_data)
  {
    writer.write("worldmap-sprite", worldmap_sprite);
    writer.write("last-worldmap", last_worldmap);
    writer.write("title-level", title_level);
  }
}

void
PlayerStatus::read(const ReaderMapping& mapping, const std::string& self_username, const std::string& remote_username)
{
  reset();

  auto iter = mapping.get_iter();
  while (iter.next())
  {
    try
    {
      if (iter.get_key().size() > 3 && iter.get_key().substr(0, 3) == "tux")
      {
        const std::string postfix = iter.get_key().substr(3);
        if (postfix == "_remote")
        {
          auto iter_mapping = iter.as_mapping();

          std::string username;
          iter_mapping.get("username", username);
          if (username.empty())
            throw std::runtime_error("Remote player has no \"username\" set!");

          int id = -1;
          iter_mapping.get("id", id);
          if (id < 0)
            throw std::runtime_error("Remote player has no \"id\" set!");

          if (self_username.empty() || username != self_username)
          {
            auto& players = m_remote_players[username];
            if (id >= players.size())
              expand(players, id);

            players[id - 1]->parse(iter.as_mapping());
          }
          else
          {
            if (id >= m_local_players.size())
              expand(m_local_players, id);

            m_local_players[id - 1]->parse(iter.as_mapping());
          }
        }
        else
        {
          const int id = std::stoi(postfix);
          if (id == 1)
          {
            throw std::runtime_error("Refusing to parse player 1 when reading player state,"
              "please don't put player 1 data in a (tux1 ...)"
              "wrapper for retrocompatibiility");
          }

          if (remote_username.empty())
          {
            if (id >= m_local_players.size())
              expand(m_local_players, id);

            m_local_players[id - 1]->parse(iter.as_mapping());
          }
          else
          {
            auto& players = m_remote_players[remote_username];
            if (id >= players.size())
              expand(players, id);

            players[id - 1]->parse(iter.as_mapping());
          }
        }
      }
    }
    catch (const std::exception& e)
    {
      log_warning << "Couldn't parse player from player status save: " << e.what() << std::endl;
    }
  }

  // Parse first player.
  if (remote_username.empty())
  {
    m_local_players[0]->parse(mapping);
  }
  else
  {
    auto& players = m_remote_players[remote_username];
    if (players.empty())
      expand(players, 1);

    players[0]->parse(mapping);
  }

  mapping.get("coins", coins);

  mapping.get("worldmap-sprite", worldmap_sprite);
  mapping.get("last-worldmap", last_worldmap);
  mapping.get("title-level", title_level);
}

void
PlayerStatus::add_local_player(int id)
{
  if (id >= get_num_local_players())
    expand(m_local_players, id + 1);
}

void
PlayerStatus::add_remote_player(const std::string& username, int id)
{
  auto& players = m_remote_players[username];
  if (id >= static_cast<int>(players.size()))
    expand(players, id + 1);
}

void
PlayerStatus::remove_local_player(int id)
{
  assert(id < get_num_local_players());
  m_local_players.erase(m_local_players.begin() + id);
}

void
PlayerStatus::remove_remote_player(const std::string& username, int id)
{
  auto& players = m_remote_players[username];
  assert(id < static_cast<int>(players.size()));
  players.erase(players.begin() + id);
}

/* EOF */
