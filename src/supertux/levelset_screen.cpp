//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/levelset_screen.hpp"

#include "editor/editor.hpp"
#include "supertux/game_session.hpp"
#include "supertux/globals.hpp"
#include "supertux/levelset.hpp"
#include "supertux/savegame.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "util/file_system.hpp"

LevelsetScreen::LevelsetScreen(const std::string& basedir, const std::string& level_filename,
                               Savegame& savegame) :
  m_basedir(basedir),
  m_level_filename(level_filename),
  m_savegame(savegame),
  m_level_started(false),
  m_solved(false)
{
  Levelset levelset(basedir);
  for(int i = 0; i < levelset.get_num_levels(); ++i)
  {
    std::string lev = levelset.get_level_filename(i);
    m_savegame.set_levelset_state(m_basedir, lev, false);
  }

  LevelsetState state = m_savegame.get_levelset_state(basedir);
  LevelState level_state = state.get_level_state(level_filename);
  m_solved = level_state.solved;
}

LevelsetScreen::~LevelsetScreen()
{
}

void
LevelsetScreen::draw(DrawingContext&)
{
}

void
LevelsetScreen::update(float elapsed_time)
{
}

void
LevelsetScreen::finished_level(bool win)
{
  m_solved = m_solved || win;
}

void
LevelsetScreen::setup()
{
  if (m_level_started)
  {
    log_info << "Saving Levelset state" << std::endl;
    // this gets called when the GameSession is done and we return back to the
    m_savegame.set_levelset_state(m_basedir, m_level_filename, m_solved);
    m_savegame.save();
    ScreenManager::current()->pop_screen();
  }
  else
  {
    m_level_started = true;

    if (Editor::is_active()) {
      log_warning << "Editor is still active, quiting Levelset screen" << std::endl;
      ScreenManager::current()->pop_screen();
    } else {
      std::unique_ptr<Screen> screen(new GameSession(FileSystem::join(m_basedir, m_level_filename),
                                                     m_savegame));
      ScreenManager::current()->push_screen(std::move(screen));
    }
  }
}

void
LevelsetScreen::leave()
{
}

/* EOF */
