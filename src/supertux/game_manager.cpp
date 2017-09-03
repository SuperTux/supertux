//  SuperTux
//  Copyright (C) 2013 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/game_manager.hpp"

#include <sstream>

#include "gui/menu_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/levelset_screen.hpp"
#include "supertux/savegame.hpp"
#include "supertux/screen.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "worldmap/worldmap.hpp"

GameManager::GameManager() :
  m_world(),
  m_savegame(),
  next_worldmap(),
  next_spawnpoint()
{
}

GameManager::~GameManager()
{
}

void
GameManager::run_level(World* world, const std::string& level_filename)
{
  m_savegame.reset(new Savegame(world->get_savegame_filename()));
  m_savegame->load();

  std::unique_ptr<Screen> screen(new LevelsetScreen(world->get_basedir(),
                                                    level_filename,
                                                    *m_savegame));
  ScreenManager::current()->push_screen(std::move(screen));
}

void
GameManager::run_worldmap(World* world, const std::string& worldmap_filename, const std::string& spawnpoint)
{
  try
  {
    m_savegame.reset(new Savegame(world->get_savegame_filename()));
    m_savegame->load();

    auto filename = m_savegame->get_player_status()->last_worldmap;
    // If we specified a worldmap filename manually,
    // this overrides the default choice of "last worldmap"
    if(!worldmap_filename.empty())
    {
      filename = worldmap_filename;
    }

    // No "last worldmap" found and no worldmap_filename
    // specified. Let's go ahead and use the worldmap
    // filename specified in the world.
    if(filename.empty())
    {
      filename = world->get_worldmap_filename();
    }

    auto worldmap = new worldmap::WorldMap(filename, *m_savegame,
                                           spawnpoint.empty() ? "main" : spawnpoint);
    ScreenManager::current()->push_screen(std::unique_ptr<Screen>(worldmap));
  }
  catch(std::exception& e)
  {
    log_fatal << "Couldn't start world: " << e.what() << std::endl;
  }
}

void
GameManager::start_level(std::unique_ptr<World> world, const std::string& level_filename)
{
  m_world = std::move(world);
  run_level(m_world.get(), level_filename);
}

void
GameManager::start_level(World* world, const std::string& level_filename)
{
  run_level(world, level_filename);
}

void
GameManager::start_worldmap(std::unique_ptr<World> world, const std::string& spawnpoint, const std::string& worldmap_filename)
{
  m_world = std::move(world);
  run_worldmap(m_world.get(), worldmap_filename, spawnpoint);
}

void
GameManager::start_worldmap(World* world, const std::string& spawnpoint, const std::string& worldmap_filename)
{
  run_worldmap(world, worldmap_filename, spawnpoint);
}

std::string
GameManager::get_level_name(const std::string& filename) const
{
  try
  {
    register_translation_directory(filename);
    auto doc = ReaderDocument::parse(filename);
    auto root = doc.get_root();

    if(root.get_name() != "supertux-level") {
      return "";
    } else {
      auto mapping = root.get_mapping();
      std::string name;
      mapping.get("name", name);
      return name;
    }
  }
  catch(const std::exception& e)
  {
    log_warning << "Problem getting name of '" << filename << "': "
                << e.what() << std::endl;
    return "";
  }
}

bool
GameManager::load_next_worldmap()
{
  if (next_worldmap.empty())
  {
    return false;
  }
  std::unique_ptr<World> world = World::load(next_worldmap);
  next_worldmap = "";
  if (!world)
  {
    log_warning << "Can't load world '" << next_worldmap << "'" <<  std::endl;
    return false;
  }
  start_worldmap(std::move(world), next_spawnpoint); // New world, new savegame
  return true;
}

void
GameManager::set_next_worldmap(const std::string& worldmap, const std::string &spawnpoint)
{
  next_worldmap = worldmap;
  next_spawnpoint = spawnpoint;
}

/* EOF */
