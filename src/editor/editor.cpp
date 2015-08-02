//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#include "editor/editor.hpp"

#include "control/input_manager.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/editor_levelset_select_menu.hpp"
#include "supertux/level.hpp"
#include "supertux/levelset_screen.hpp"
#include "supertux/savegame.hpp"
#include "supertux/fadein.hpp"
#include "supertux/screen.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"

Editor::Editor() :
  level(),
  levelset(),
  levelfile(),
  quit_request(false),
  newlevel_request(false),
  reload_request(false),
  currentsector(),
  levelloaded(false)
{
}

Editor::~Editor()
{

}

void Editor::draw(DrawingContext& context)
{
  if (levelloaded) {
    currentsector->draw(context);
  }
}

void Editor::update(float elapsed_time)
{
  if (reload_request) {
    reload_level();
  }

  if (quit_request) {
    quit_editor();
  }

  if (newlevel_request) {
    //Create new level
  }

  if (InputManager::current()->get_controller()->pressed(Controller::ESCAPE)) {
    quit_request = true;
  }
}

void Editor::reload_level() {
  reload_request = false;
  // Re/load level
  if (!levelloaded) {
    level.reset(new Level);
    levelloaded = true;
  }
  level->load(levelset + "/" + levelfile);
  currentsector = level->get_sector("main");
  if(!currentsector)
    throw std::runtime_error("Couldn't find main sector");
  currentsector->activate("main");
}

void Editor::quit_editor() {
  //Quit level editor
  levelset = "";
  levelfile = "";
  levelloaded = false;
  quit_request = false;
  ScreenManager::current()->pop_screen();
}

void Editor::leave()
{

}

void
Editor::setup() {
  if (levelset == "") {
    MenuManager::instance().set_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
  }
//  GameSession::current() = this;
}
