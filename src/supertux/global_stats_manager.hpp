//  SuperTux
//  Copyright (C) 2026 SuperTux Team
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

#pragma once

#include <memory>
#include <string>
#include <vector>

class World;

class GlobalStatsManager final
{
public:
  struct Summary final
  {
    Summary();

    int worlds;
    int total_coins;
    int total_secrets;
    int total_tuxdolls;
    int solved_levels;
    int total_levels;
    int perfect_levels;
    float total_time;
  };

public:
  GlobalStatsManager();

  Summary aggregate() const;

private:
  std::vector<std::unique_ptr<World> > collect_worlds() const;
  void collect_worlds_from_directory(const std::string& directory, std::vector<std::string>& world_dirs) const;
  void aggregate_world(const World& world, Summary& summary) const;

private:
  GlobalStatsManager(const GlobalStatsManager&) = delete;
  GlobalStatsManager& operator=(const GlobalStatsManager&) = delete;
};
