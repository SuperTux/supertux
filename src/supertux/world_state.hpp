//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2014 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_WORLD_STATE_HPP
#define HEADER_SUPERTUX_SUPERTUX_WORLD_STATE_HPP

#include <string>
#include <memory>

class PlayerStatus;

class WorldState
{
private:
  std::unique_ptr<PlayerStatus> m_player_status;

public:
  WorldState();

  PlayerStatus* get_player_status() const { return m_player_status.get(); }

  void save(const std::string& filename);
  void load(const std::string& filename);

private:
  WorldState(const WorldState&) = delete;
  WorldState& operator=(const WorldState&) = delete;
};

#endif

/* EOF */
