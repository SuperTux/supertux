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

#include "supertux/global_stats_manager.hpp"

#include <algorithm>
#include <set>

#include <physfs.h>

#include "physfs/util.hpp"
#include "supertux/levelset.hpp"
#include "supertux/savegame.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "worldmap/worldmap.hpp"

namespace {

void aggregate_level_state(const LevelState& level_state, GlobalStatsManager::Summary& summary)
{
  if (level_state.has_statistics)
  {
    summary.total_coins += level_state.coins;
    summary.total_secrets += level_state.secrets;
    summary.total_tuxdolls += level_state.tuxdolls;
  }

  if (level_state.solved)
  {
    summary.solved_levels++;
    if (level_state.has_statistics)
      summary.total_time += level_state.time;
  }

  if (level_state.perfect)
    summary.perfect_levels++;
}

} // namespace

GlobalStatsManager::Summary::Summary() :
  worlds(0),
  total_coins(0),
  total_secrets(0),
  total_tuxdolls(0),
  solved_levels(0),
  total_levels(0),
  perfect_levels(0),
  total_time(0.f)
{
}

GlobalStatsManager::GlobalStatsManager()
{
}

GlobalStatsManager::Summary
GlobalStatsManager::aggregate() const
{
  Summary summary;

  for (const auto& world : collect_worlds())
  {
    aggregate_world(*world, summary);
  }

  return summary;
}

std::vector<std::unique_ptr<World> >
GlobalStatsManager::collect_worlds() const
{
  std::vector<std::string> world_dirs;
  collect_worlds_from_directory("levels", world_dirs);

  physfsutil::enumerate_files_alphabetical("custom", [this, &world_dirs](const std::string& addon_filename) {
    const std::string addon_path = FileSystem::join("custom", addon_filename);
    const std::string addon_levels_path = FileSystem::join(addon_path, "levels");

    if (physfsutil::is_directory(addon_levels_path))
      collect_worlds_from_directory(addon_levels_path, world_dirs);

    return false;
  });

  std::set<std::string> unique_dirs(world_dirs.begin(), world_dirs.end());
  std::vector<std::unique_ptr<World> > worlds;

  for (const auto& world_dir : unique_dirs)
  {
    try
    {
      std::unique_ptr<World> world = World::from_directory(world_dir);
      if (world->is_worldmap())
      {
        const std::string worldmap_filename = world->get_worldmap_filename();
        if (PHYSFS_exists(worldmap_filename.c_str()) || FileSystem::exists(worldmap_filename))
          worlds.push_back(std::move(world));
      }
      else
      {
        worlds.push_back(std::move(world));
      }
    }
    catch (const std::exception& err)
    {
      log_warning << "Failed to load world info from '" << world_dir << "': " << err.what() << std::endl;
    }
  }

  return worlds;
}

void
GlobalStatsManager::collect_worlds_from_directory(const std::string& directory, std::vector<std::string>& world_dirs) const
{
  physfsutil::enumerate_files_alphabetical(directory, [&directory, &world_dirs](const std::string& filename) {
    const std::string filepath = FileSystem::join(directory, filename);
    if (physfsutil::is_directory(filepath))
      world_dirs.push_back(filepath);

    return false;
  });
}

void
GlobalStatsManager::aggregate_world(const World& world, Summary& summary) const
{
  try
  {
    std::unique_ptr<Savegame> savegame = Savegame::from_current_profile(world.get_basename());

    if (world.is_worldmap())
    {
      const std::string worldmap_filename = physfsutil::realpath(world.get_worldmap_filename());
      worldmap::WorldMap worldmap(worldmap_filename, *savegame);
      summary.total_levels += static_cast<int>(worldmap.level_count());

      WorldmapState state = savegame->get_worldmap_state(worldmap_filename);
      for (const auto& level_state : state.level_states)
        aggregate_level_state(level_state, summary);
    }
    else
    {
      Levelset levelset(world.get_basedir());
      summary.total_levels += levelset.get_num_levels();

      LevelsetState state = savegame->get_levelset_state(world.get_basedir());
      for (const auto& level_state : state.level_states)
        aggregate_level_state(level_state, summary);
    }

    summary.worlds++;
  }
  catch (const std::exception& err)
  {
    log_warning << "Failed to aggregate statistics for '" << world.get_basedir() << "': " << err.what() << std::endl;
  }
}
