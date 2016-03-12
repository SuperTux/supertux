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
#include "editor/layer_icon.hpp"
#include "gui/mousecursor.hpp"
#include "object/camera.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/editor_menu.hpp"
#include "supertux/menu/editor_levelset_select_menu.hpp"
#include "supertux/game_object.hpp"
#include "supertux/level.hpp"
#include "supertux/levelset_screen.hpp"
#include "supertux/savegame.hpp"
#include "supertux/fadein.hpp"
#include "supertux/screen.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
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
  tileselect(),
  layerselect(),
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
  tileselect.draw(context);
  layerselect.draw(context);
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
    save_request = false;
  }

  if (InputManager::current()->get_controller()->pressed(Controller::ESCAPE)) {
    enabled = false;
    MenuManager::instance().set_menu(MenuStorage::EDITOR_MENU);
  }

  if (levelloaded) {
    currentsector->update(0);
  }

  update_keyboard();
  tileselect.update(elapsed_time);
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

void Editor::load_layers() {
  layerselect.layers.clear();
  for(auto i = currentsector->gameobjects.begin(); i != currentsector->gameobjects.end(); i++) {
    GameObject* go = &(**i);
    if (go->get_class() == "tilemap") {
      layerselect.add_layer(go,"images/engine/editor/tilemap.png");
      continue;
    }
    if (go->get_class() == "background") {
      layerselect.add_layer(go,"images/engine/editor/background.png");
      continue;
    }
    if (go->get_class() == "gradient") {
      layerselect.add_layer(go,"images/engine/editor/gradient.png");
      continue;
    }
    if (go->get_class() == "camera") {
      layerselect.add_layer(go,"images/engine/editor/camera.png");
      continue;
    }
    if (go->get_class() == "leveltime") {
      layerselect.add_layer(go,"images/engine/editor/clock.png");
      continue;
    }
    if (go->get_class() == "particle-clouds") {
      layerselect.add_layer(go,"images/engine/editor/clouds.png");
      continue;
    }
    if (go->get_class() == "particle-rain") {
      layerselect.add_layer(go,"images/engine/editor/rain.png");
      continue;
    }
    if (go->get_class() == "particle-ghosts") {
      layerselect.add_layer(go,"images/engine/editor/ghostparticles.png");
      continue;
    }
    if (go->get_class() == "particle-snow") {
      layerselect.add_layer(go,"images/engine/editor/snow.png");
      continue;
    }
    if (go->get_class() == "thunderstorm") {
      layerselect.add_layer(go,"images/engine/editor/thunderstorm.png");
      continue;
    }

  }
  layerselect.refresh_sector_text();
}

void Editor::load_sector(std::string name) {
  currentsector = level->get_sector(name);
  if(!currentsector) {
    size_t i = 0;
    currentsector = level->get_sector(i);
  }
  load_layers();
}

void Editor::load_sector(int id) {
  size_t i = id;
  currentsector = level->get_sector(i);
  load_layers();
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
  load_sector("main");
  currentsector->activate("main");
  currentsector->camera->mode = Camera::MANUAL;
  layerselect.refresh_sector_text();
}

void Editor::quit_editor() {
  //Quit level editor
  world = NULL;
  levelfile = "";
  levelloaded = false;
  quit_request = false;
  enabled = false;
  Tile::draw_editor_images = false;
  ScreenManager::current()->pop_screen();
}

void Editor::leave()
{

}

void
Editor::setup() {
  Tile::draw_editor_images = true;
  Sector::draw_solids_only = false;
  MenuManager::instance().set_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
  tileselect.setup();
  layerselect.setup();
}

void
Editor::resize() {
  tileselect.resize();
  layerselect.resize();
}

void
Editor::event(SDL_Event& ev) {
  if ( tileselect.event(ev) ) {
    return;
  }

  if ( layerselect.event(ev) ) {
    return;
  }

}
