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

#include <limits>

#include "addon/addon_manager.hpp"
#include "audio/sound_manager.hpp"
#include "control/input_manager.hpp"
#include "editor/layer_icon.hpp"
#include "gui/dialog.hpp"
#include "gui/mousecursor.hpp"
#include "gui/menu_manager.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "object/tilemap.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/editor_menu.hpp"
#include "supertux/menu/editor_levelset_select_menu.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_object.hpp"
#include "supertux/level.hpp"
#include "supertux/level_parser.hpp"
#include "supertux/levelset_screen.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/savegame.hpp"
#include "supertux/fadein.hpp"
#include "supertux/screen.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/reader_mapping.hpp"
#include "video/surface.hpp"

Editor::Editor() :
  level(),
  world(),
  levelfile(),
  worldmap_mode(false),
  quit_request(false),
  newlevel_request(false),
  reload_request(false),
  reactivate_request(false),
  deactivate_request(false),
  save_request(false),
  test_request(false),
  m_savegame(),
  currentsector(),
  levelloaded(false),
  leveltested(false),
  tileset(NULL),
  inputcenter(),
  tileselect(),
  layerselect(),
  scroller(),
  enabled(false),
  bgr_surface(Surface::create("images/background/forest1.jpg"))
{
}

Editor::~Editor()
{

}

void Editor::draw(DrawingContext& context)
{
  if (levelloaded) {
    currentsector->draw(context);
    context.draw_filled_rect(Rectf(Vector(0, 0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT)), Color(0.0f, 0.0f, 0.0f),
                             0.0f, std::numeric_limits<int>::min());
  } else {
    context.draw_surface_part(bgr_surface, Rectf(Vector(0, 0), bgr_surface->get_size()),
                              Rectf(Vector(0, 0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT)), -100);
  }
  inputcenter.draw(context);
  tileselect.draw(context);
  layerselect.draw(context);
  scroller.draw(context);
  MouseCursor::current()->draw(context);
}

void Editor::update(float elapsed_time)
{
  // Pass all requests
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
    level->save(world ? FileSystem::join(world->get_basedir(), levelfile) :
                         levelfile);
    enabled = true;
    save_request = false;
  }

  if (test_request) {
    test_request = false;
    MouseCursor::current()->set_icon(NULL);
    test_level();
    return;
  }

  if (deactivate_request) {
    enabled = false;
    deactivate_request = false;
    return;
  }

  // update other stuff
  if (is_active()) {
    currentsector->update(0);
    tileselect.update(elapsed_time);
    layerselect.update(elapsed_time);
    inputcenter.update(elapsed_time);
    scroller.update(elapsed_time);
    update_keyboard();
  }
}

void Editor::test_level() {
  Tile::draw_editor_images = false;
  level->save("levels/misc/test.stl");
  std::unique_ptr<World> test_world = World::load("levels/misc");
  GameManager::current()->start_level(std::move(test_world), "test.stl");
  leveltested = true;
}

bool Editor::can_scroll_vert() const {
  return levelloaded && (currentsector->get_height() + 32 > SCREEN_HEIGHT);
}

bool Editor::can_scroll_horz() const {
  return levelloaded && (currentsector->get_width() + 128 > SCREEN_WIDTH);
}

void Editor::scroll_left(float speed) {
  auto camera = currentsector->camera;
  if (can_scroll_horz()) {
    if (camera->get_translation().x >= speed*32) {
      camera->move(-32 * speed, 0);
    } else {
      //When is the camera less than one tile after the left limit, it puts the camera to the limit.
      camera->move(-camera->get_translation().x, 0);
    }
    inputcenter.update_pos();
  }
}

void Editor::scroll_right(float speed) {
  auto camera = currentsector->camera;
  if (can_scroll_horz()) {
    if (camera->get_translation().x <= currentsector->get_width() - SCREEN_WIDTH + 128 - 32 * speed) {
      camera->move(32*speed, 0);
    } else {
      //When is the camera less than one tile after the right limit, it puts the camera to the limit.
      // The limit is shifted 128 pixels to the right due to the input gui.
      camera->move(currentsector->get_width() - camera->get_translation().x - SCREEN_WIDTH +128, 0);
    }
    inputcenter.update_pos();
  }
}

void Editor::scroll_up(float speed) {
  auto camera = currentsector->camera;
  if (can_scroll_vert()) {
    if (camera->get_translation().y >= speed*32) {
      camera->move(0,-32*speed);
    } else {
      //When is the camera less than one tile after the top limit, it puts the camera to the limit.
      camera->move(0, -camera->get_translation().y);
    }
    inputcenter.update_pos();
  }
}

void Editor::scroll_down(float speed) {
  auto camera = currentsector->camera;
  if (can_scroll_vert()) {
    if (camera->get_translation().y <= currentsector->get_height() - SCREEN_HEIGHT - 32 * speed) {
      camera->move(0, 32*speed);
    } else {
      //When is the camera less than one tile after the bottom limit, it puts the camera to the limit.
      // The limit is shifted 32 pixels to the bottom due to the layer toolbar.
      camera->move(0, currentsector->get_height() - camera->get_translation().y - SCREEN_HEIGHT +32);
    }
    inputcenter.update_pos();
  }
}

void Editor::esc_press() {
  enabled = false;
  inputcenter.delete_markers();
  MenuManager::instance().set_menu(MenuStorage::EDITOR_MENU);
}

void Editor::update_keyboard() {

  if (!enabled){
    return;
  }

  auto controller = InputManager::current()->get_controller();

  if (controller->pressed(Controller::ESCAPE)) {
    esc_press();
    return;
  }

  if (controller->hold(Controller::LEFT)) {
    scroll_left();
  }

  if (controller->hold(Controller::RIGHT)) {
    scroll_right();
  }

  if (controller->hold(Controller::UP)) {
    scroll_up();
  }

  if (controller->hold(Controller::DOWN)) {
    scroll_down();
  }
}

void Editor::load_layers() {
  layerselect.selected_tilemap = NULL;
  layerselect.layers.clear();
  bool tsel = false;
  for(auto& i : currentsector->gameobjects) {
    auto go = i.get();
    auto mo = dynamic_cast<MovingObject*>(go);
    if ( !mo && go->do_save() ) {
      layerselect.add_layer(go);

      TileMap *tm = dynamic_cast<TileMap*>(go);
      if (tm) {
        if ( !tm->is_solid() || tsel ) {
          tm->editor_active = false;
        } else {
          layerselect.selected_tilemap = tm;
          tm->editor_active = true;
          tsel = true;
        }
      }

    }
  }

  layerselect.refresh_sector_text();
}

void Editor::load_sector(const std::string& name) {
  currentsector = level->get_sector(name);
  if(!currentsector) {
    size_t i = 0;
    currentsector = level->get_sector(i);
  }
  currentsector->activate("main");
  load_layers();
}

void Editor::load_sector(int id) {
  size_t i = id;
  currentsector = level->get_sector(i);
  currentsector->activate("main");
  load_layers();
}

void Editor::reload_level() {
  reload_request = false;
  enabled = true;
  tileselect.input_type = EditorInputGui::IP_NONE;
  // Re/load level
  level = NULL;
  levelloaded = true;

  ReaderMapping::translations_enabled = false;
  level = LevelParser::from_file(world ? FileSystem::join(world->get_basedir(),
                                                          levelfile) : levelfile);
  ReaderMapping::translations_enabled = true;

  tileset = TileManager::current()->get_tileset(level->get_tileset());
  load_sector("main");
  currentsector->activate("main");
  currentsector->camera->mode = Camera::MANUAL;
  layerselect.refresh_sector_text();
  tileselect.update_mouse_icon();
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
  MouseCursor::current()->set_icon(NULL);
}

void
Editor::setup() {
  Tile::draw_editor_images = true;
  Sector::draw_solids_only = false;
  if (!levelloaded) {

#if 0
    if (AddonManager::current()->is_old_addon_enabled()) {
      std::unique_ptr<Dialog> dialog(new Dialog);
      dialog->set_text(_("Some obsolete add-ons are still active\nand might cause collisions with default Super Tux structure.\nYou can still enable these add-ons in the menu.\nDisabling these add-ons will not delete your game progress."));
      dialog->clear_buttons();

      dialog->add_default_button(_("Disable add-ons"), [] {
        AddonManager::current()->disable_old_addons();
        MenuManager::instance().push_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
      });

      dialog->add_button(_("Ignore (not advised)"), [] {
        MenuManager::instance().push_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
      });

      dialog->add_button(_("Leave editor"), [this] {
        this->quit_request = true;
      });

      MenuManager::instance().set_dialog(std::move(dialog));
    } else
#endif
    {
      MenuManager::instance().push_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
    }
  }
  tileselect.setup();
  layerselect.setup();
  m_savegame.reset(new Savegame("levels/misc"));
  m_savegame->load();

  // Reactivate the editor after level test
  if (leveltested) {
    leveltested = false;
    Tile::draw_editor_images = true;
    level->reactivate();
    currentsector->activate(currentsector->player->get_pos());
    MenuManager::instance().clear_menu_stack();
    SoundManager::current()->stop_music();
    deactivate_request = false;
    enabled = true;
    tileselect.update_mouse_icon();
  }
}

void
Editor::resize() {
  // Calls on window resize.
  tileselect.resize();
  layerselect.resize();
  inputcenter.update_pos();
}

void
Editor::event(SDL_Event& ev) {
  if (enabled) {
    if ( tileselect.event(ev) ) {
      return;
    }

    if ( layerselect.event(ev) ) {
      return;
    }

    if ( scroller.event(ev) ) {
      return;
    }
    inputcenter.event(ev);
  }
}

bool
Editor::is_active() {
  auto self = Editor::current();
  return self && self->levelloaded && !self->leveltested;
}

void
Editor::update_node_iterators() {
  inputcenter.update_node_iterators();
}

void
Editor::delete_markers() {
  inputcenter.delete_markers();
}

void
Editor::sort_layers() {
  layerselect.sort_layers();
}

/* EOF */
