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

#ifndef HEADER_SUPERTUX_SUPERTUX_PLAYER_STATUS_HPP
#define HEADER_SUPERTUX_SUPERTUX_PLAYER_STATUS_HPP

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class DrawingContext;
class ReaderMapping;
class Writer;

static const float BORDER_X = 10;
static const float BORDER_Y = 10;

enum BonusType {
  NO_BONUS = 0, GROWUP_BONUS, FIRE_BONUS, ICE_BONUS, AIR_BONUS, EARTH_BONUS
};

/** This class keeps player status between different game sessions (for
    example when switching maps in the worldmap) */
class PlayerStatus final
{
public:
  class Status final
  {
  private:
    Status(const PlayerStatus::Status& other);

  public:
    Status(PlayerStatus& status);

    void save_state();
    void restore_state();

    void parse(const ReaderMapping& reader);
    void write(Writer& writer) const;

    std::string get_bonus_prefix() const; /** Returns the prefix of the animations that should be displayed */
    bool has_hat_sprite() const { return bonus > GROWUP_BONUS; }

  public:
    PlayerStatus& general_status;

    BonusType bonus;
    int max_fire_bullets; /**< maximum number of fire bullets in play */
    int max_ice_bullets; /**< maximum number of ice bullets in play */
    int max_air_time; /**<determines maximum number of seconds player can float in air */
    int max_earth_time; /**< determines maximum number of seconds player can turn to stone */

  private:
    std::unique_ptr<Status> m_saved_state;
  };

public:
  PlayerStatus();

  void save_state();
  void restore_state();

  void add_coins(int count, bool play_sound = true);
  void take_checkpoint_coins();

  std::string write(bool include_world_data = true) const;
  void read(const std::string& data, const std::string& self_username = "", const std::string& remote_username = "");

  void write(Writer& writer, bool include_world_data = true) const;
  void read(const ReaderMapping& mapping, const std::string& self_username = "", const std::string& remote_username = "");

  int get_max_coins() const;
  bool can_reach_checkpoint() const;
  bool respawns_at_checkpoint() const;

  void add_local_player(int id);
  void add_remote_player(const std::string& username, int id);
  void remove_local_player(int id);
  void remove_remote_player(const std::string& username, int id);

  Status& get_local_player(int id) { return *m_local_players[id]; }
  Status& get_remote_player(const std::string& username, int id) { return *m_remote_players[username][id]; }

  int get_num_local_players() const { return static_cast<int>(m_local_players.size()); }
  int get_num_remote_players() const { return static_cast<int>(m_remote_players.size()); }
  int get_num_players() const { return get_num_local_players() + get_num_remote_players(); }

private:
  void reset();
  void expand(std::vector<std::unique_ptr<Status>>& vec, int n);

private:
  std::vector<std::unique_ptr<Status>> m_local_players;
  std::unordered_map<std::string, std::vector<std::unique_ptr<Status>>> m_remote_players;

  int m_saved_coins;

public:
  int coins;

  std::string worldmap_sprite; /**< the sprite of Tux that should be used in worldmap */
  std::string last_worldmap; /**< the last played worldmap */
  std::string title_level; /**< level to be used for the title screen, overrides the value of the same property for the world */

private:
  PlayerStatus(const PlayerStatus&) = delete;
  PlayerStatus& operator=(const PlayerStatus&) = delete;
};

#endif

/* EOF */
