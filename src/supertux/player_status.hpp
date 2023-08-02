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
  PlayerStatus(int num_players);
  void reset(int num_players);
  void add_coins(int count, bool play_sound = true);
  void take_checkpoint_coins();

  void write(Writer& writer);
  void read(const ReaderMapping& mapping);

  int get_max_coins() const;
  bool can_reach_checkpoint() const;
  bool respawns_at_checkpoint() const;
  std::string get_bonus_prefix(int player_id) const;/**Returns the prefix of the animations that should be displayed*/
  bool has_hat_sprite(int player_id) const { return bonus[player_id] > GROWUP_BONUS; }

  void add_player();
  void remove_player(int player_id);

private:
  void parse_bonus_mapping(const ReaderMapping& map, int id);

public:
  int m_num_players;

  int coins;
  std::vector<BonusType> bonus;
  std::vector<int> max_fire_bullets; /**< maximum number of fire bullets in play */
  std::vector<int> max_ice_bullets; /**< maximum number of ice bullets in play */
  std::vector<int> max_air_time; /**<determines maximum number of seconds player can float in air */
  std::vector<int> max_earth_time; /**< determines maximum number of seconds player can turn to stone */

  std::string worldmap_sprite; /**< the sprite of Tux that should be used in worldmap */
  std::string last_worldmap; /**< the last played worldmap */

private:
  PlayerStatus(const PlayerStatus&) = delete;
  PlayerStatus& operator=(const PlayerStatus&) = delete;
};

#endif

/* EOF */
