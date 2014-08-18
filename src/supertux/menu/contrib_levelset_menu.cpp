//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/menu/contrib_levelset_menu.hpp"

#include <sstream>

#include "audio/sound_manager.hpp"
#include "gui/menu_item.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/levelset.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

ContribLevelsetMenu::ContribLevelsetMenu(std::unique_ptr<World> world) :
  m_world(std::move(world)),
  m_levelset()
{
  assert(m_world->is_levelset());

  m_levelset = std::unique_ptr<Levelset>(new Levelset(m_world->get_basedir()));

  Savegame savegame(m_world->get_savegame_filename());
  savegame.load();
  LevelsetState state = savegame.get_levelset_state(m_world->get_basedir());

  add_label(m_world->get_title());
  add_hl();

  for (int i = 0; i < m_levelset->get_num_levels(); ++i)
  {
    std::string filename = m_levelset->get_level_filename(i);
    std::string full_filename = FileSystem::join(m_world->get_basedir(), filename);
    std::string title = GameManager::current()->get_level_name(full_filename);
    LevelState level_state = state.get_level_state(filename);

    std::ostringstream out;
    if (level_state.solved)
    {
      out << title << " [*]";
    }
    else
    {
      out << title << " [ ]";
    }
    add_entry(i, out.str());
  }

  add_hl();
  add_back(_("Back"));
}

void
ContribLevelsetMenu::menu_action(MenuItem* item)
{
  if (item->kind == MN_ACTION)
  {
    SoundManager::current()->stop_music();

    // reload the World so that we have something that we can safely
    // std::move() around without wreaking the ContribMenu
    std::unique_ptr<World> world = World::load(m_world->get_basedir());
    GameManager::current()->start_level(std::move(world), m_levelset->get_level_filename(item->id));
  }
}

/* EOF */
