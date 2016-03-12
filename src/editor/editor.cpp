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
#include "gui/mousecursor.hpp"
#include "object/camera.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/editor_menu.hpp"
#include "supertux/menu/editor_levelset_select_menu.hpp"
#include "supertux/level.hpp"
#include "supertux/levelset_screen.hpp"
#include "supertux/savegame.hpp"
#include "supertux/fadein.hpp"
#include "supertux/screen.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/world.hpp"

Editor::Editor() :
  level(),
  world(),
  levelfile(),
  quit_request(false),
  newlevel_request(false),
  reload_request(false),
  reactivate_request(false),
  save_request(false),
  currentsector(),
  levelloaded(false),
  enabled(false)
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
  MouseCursor::current()->draw(context);
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

  if (reactivate_request) {
    enabled = true;
    reactivate_request = false;
  }

  if (save_request) {
    level->save(world->get_basedir() + "/" + levelfile);
  }

  if (InputManager::current()->get_controller()->pressed(Controller::ESCAPE)) {
    enabled = false;
    MenuManager::instance().set_menu(MenuStorage::EDITOR_MENU);
  }

  update_keyboard();
}


void Editor::update_keyboard() {
  if (!enabled){
    return;
  }

  if (InputManager::current()->get_controller()->hold(Controller::LEFT)) {
    currentsector->camera->move(-32,0);
  }

  if (InputManager::current()->get_controller()->hold(Controller::RIGHT)) {
    currentsector->camera->move(32,0);
  }

  if (InputManager::current()->get_controller()->hold(Controller::UP)) {
    currentsector->camera->move(0,-32);
  }

  if (InputManager::current()->get_controller()->hold(Controller::DOWN)) {
    currentsector->camera->move(0,32);
  }
}

void Editor::reload_level() {
  reload_request = false;
  enabled = true;
  // Re/load level
  if (levelloaded) {
    level = NULL;
  }
  level.reset(new Level);
  levelloaded = true;
  level->load(world->get_basedir() + "/" + levelfile);
  currentsector = level->get_sector("main");
  if(!currentsector) {
    size_t i = 0;
    currentsector = level->get_sector(i);
  }
  currentsector->activate("main");
  currentsector->camera->mode = Camera::MANUAL;
}

void Editor::quit_editor() {
  //Quit level editor
  world = NULL;
  levelfile = "";
  levelloaded = false;
  quit_request = false;
  enabled = false;
  ScreenManager::current()->pop_screen();
}

void Editor::leave()
{

}

void
Editor::setup() {
  MenuManager::instance().set_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
//  GameSession::current() = this;
}
