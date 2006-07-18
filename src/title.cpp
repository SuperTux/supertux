//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>
#include <physfs.h>

#include "title.hpp"
#include "mainloop.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "audio/sound_manager.hpp"
#include "gui/menu.hpp"
#include "timer.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "level.hpp"
#include "world.hpp"
#include "game_session.hpp"
#include "worldmap/worldmap.hpp"
#include "player_status.hpp"
#include "tile.hpp"
#include "sector.hpp"
#include "object/tilemap.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "resources.hpp"
#include "gettext.hpp"
#include "textscroller.hpp"
#include "fadeout.hpp"
#include "file_system.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "control/codecontroller.hpp"
#include "main.hpp"
#include "log.hpp"
#include "options_menu.hpp"
#include "console.hpp"
#include "random_generator.hpp"

enum MainMenuIDs {
  MNID_STARTGAME,
  MNID_LEVELS_CONTRIB,
  MNID_OPTIONMENU,
  MNID_LEVELEDITOR,
  MNID_CREDITS,
  MNID_QUITMAINMENU
};

void
TitleScreen::update_load_game_menu()
{
  load_game_menu.reset(new Menu());

  load_game_menu->add_label(_("Start Game"));
  load_game_menu->add_hl();
  for(int i = 1; i <= 5; ++i) {
    load_game_menu->add_entry(i, get_slotinfo(i));
  }
  load_game_menu->add_hl();
  load_game_menu->add_back(_("Back"));
}

void
TitleScreen::free_contrib_menu()
{
  for(std::vector<World*>::iterator i = contrib_worlds.begin();
      i != contrib_worlds.end(); ++i)
    delete *i;

  contrib_worlds.clear();
}

void
TitleScreen::generate_contrib_menu()
{
  /** Generating contrib levels list by making use of Level Subset  */
  std::vector<std::string> level_worlds; 
  char** files = PHYSFS_enumerateFiles("levels/");
  for(const char* const* filename = files; *filename != 0; ++filename) {
    std::string filepath = std::string("levels/") + *filename;
    if(PHYSFS_isDirectory(filepath.c_str()))
      level_worlds.push_back(filepath);
  }
  PHYSFS_freeList(files);

  free_contrib_menu();
  contrib_menu.reset(new Menu());

  contrib_menu->add_label(_("Contrib Levels"));
  contrib_menu->add_hl();
  
  int i = 0;
  for (std::vector<std::string>::iterator it = level_worlds.begin();
      it != level_worlds.end(); ++it) {
    try {
      std::auto_ptr<World> world (new World());
      world->load(*it + "/info");
      if(world->hide_from_contribs) {
        continue;
      }
      contrib_menu->add_entry(i++, world->title);
      contrib_worlds.push_back(world.release());
    } catch(std::exception& e) {
#ifdef DEBUG
      log_warning << "Couldn't parse levelset info for '" << *it << "': " << e.what() << std::endl;
#endif
    }
  }

  contrib_menu->add_hl();
  contrib_menu->add_back(_("Back"));
}

std::string
TitleScreen::get_level_name(const std::string& filename)
{
  try {
    lisp::Parser parser;
    std::auto_ptr<lisp::Lisp> root (parser.parse(filename));

    const lisp::Lisp* level = root->get_lisp("supertux-level");
    if(!level)
      return "";

    std::string name;
    level->get("name", name);
    return name;
  } catch(std::exception& e) {
    log_warning << "Problem getting name of '" << filename << "'." << std::endl;
    return "";
  }
}

void
TitleScreen::check_levels_contrib_menu()
{
  int index = contrib_menu->check();
  if (index == -1)
    return;

  current_world = contrib_worlds[index];

  if(!current_world->is_levelset) {
    update_load_game_menu();
    Menu::push_current(load_game_menu.get());
  } else {
    contrib_world_menu.reset(new Menu());

    contrib_world_menu->add_label(current_world->title);
    contrib_world_menu->add_hl();

    for (unsigned int i = 0; i < current_world->get_num_levels(); ++i)
    {
      /** get level's title */
      std::string filename = current_world->get_level_filename(i);
      std::string title = get_level_name(filename);
      contrib_world_menu->add_entry(i, title);
    }

    contrib_world_menu->add_hl();
    contrib_world_menu->add_back(_("Back"));

    Menu::push_current(contrib_world_menu.get());
  }
}

void
TitleScreen::check_contrib_world_menu()
{
  int index = contrib_world_menu->check();
  if (index != -1) {
    if (contrib_world_menu->get_item_by_id(index).kind == MN_ACTION) {
      sound_manager->stop_music();
      GameSession* session =
        new GameSession(current_world->get_level_filename(index));
      main_loop->push_screen(session);
    }
  }  
}

void
TitleScreen::make_tux_jump()
{
  static Timer randomWaitTimer;
  static Timer jumpPushTimer;
  static float last_tux_x_pos = -1;
  static float last_tux_y_pos = -1;

  Sector* sector  = titlesession->get_current_sector();
  Player* tux = sector->player;

  //sector->play_music(LEVEL_MUSIC);

  controller->update();
  controller->press(Controller::RIGHT);

  // Determine how far we moved since last frame
  float dx = fabsf(last_tux_x_pos - tux->get_pos().x); 
  float dy = fabsf(last_tux_y_pos - tux->get_pos().y); 
 
  // Calculate space to check for obstacles 
  Rect lookahead = tux->get_bbox();
  lookahead.move(Vector(96, 0));
  
  // Check if we should press the jump button
  bool randomJump = !randomWaitTimer.started();
  bool notMoving = (fabsf(dx) + fabsf(dy)) < 0.1;
  bool pathBlocked = !sector->is_free_space(lookahead); 
  if (!controller->released(Controller::JUMP)
      && (notMoving || pathBlocked || randomJump)) {
    float jumpDuration;
    if(pathBlocked)
      jumpDuration = 0.5;
    else
      jumpDuration = systemRandom.randf(0.3, 0.8);
    jumpPushTimer.start(jumpDuration);
    randomWaitTimer.start(systemRandom.randf(3.0, 6.0));
  }

  // Keep jump button pressed
  if (jumpPushTimer.started())
    controller->press(Controller::JUMP);

  // Remember last position, so we can determine if we moved
  last_tux_x_pos = tux->get_pos().x;
  last_tux_y_pos = tux->get_pos().y;

  // Wrap around at the end of the level back to the beginnig
  if(sector->get_width() * 32 - 320 < tux->get_pos().x) {
    sector->activate("main");
    sector->camera->reset(tux->get_pos());
  }
}

TitleScreen::TitleScreen()
{
  controller.reset(new CodeController());
  titlesession.reset(new GameSession("levels/misc/menu.stl"));

  Player* player = titlesession->get_current_sector()->player;
  player->set_controller(controller.get());

  main_menu.reset(new Menu());
  main_menu->set_pos(SCREEN_WIDTH/2, 335);
  main_menu->add_entry(MNID_STARTGAME, _("Start Game"));
  main_menu->add_entry(MNID_LEVELS_CONTRIB, _("Contrib Levels"));
  main_menu->add_submenu(_("Options"), get_options_menu());
  main_menu->add_entry(MNID_CREDITS, _("Credits"));
  main_menu->add_entry(MNID_QUITMAINMENU, _("Quit"));
}

TitleScreen::~TitleScreen()
{
}

void
TitleScreen::setup()
{
  player_status->reset();

  Sector* sector = titlesession->get_current_sector();
  if(Sector::current() != sector) {
    sector->play_music(LEVEL_MUSIC);
    sector->activate(sector->player->get_pos());
  }

  Menu::set_current(main_menu.get());
}

void
TitleScreen::leave()
{
  Sector* sector = titlesession->get_current_sector();
  sector->deactivate();
  Menu::set_current(NULL);
}

void
TitleScreen::draw(DrawingContext& context)
{
  Sector* sector  = titlesession->get_current_sector();
  sector->draw(context);
 
  context.draw_text(white_small_text, " SuperTux " PACKAGE_VERSION "\n",
      Vector(0, SCREEN_HEIGHT - 50), LEFT_ALLIGN, LAYER_FOREGROUND1);
  context.draw_text(white_small_text,
      _(
"Copyright (c) 2006 SuperTux Devel Team\n"
"This game comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to\n"
"redistribute it under certain conditions; see the file COPYING for details.\n"
),
      Vector(0, SCREEN_HEIGHT - 50 + white_small_text->get_height() + 5),
      LEFT_ALLIGN, LAYER_FOREGROUND1);
}

void
TitleScreen::update(float elapsed_time)
{
  main_loop->set_speed(0.6);
  Sector* sector  = titlesession->get_current_sector();
  sector->update(elapsed_time);

  make_tux_jump();
  
  Menu* menu = Menu::current();
  if(menu) {
    menu->update();
  	  
    if(menu == main_menu.get()) {
      switch (main_menu->check()) {
        case MNID_STARTGAME:
          // Start Game, ie. goto the slots menu
          if(main_world.get() == NULL) {
            main_world.reset(new World());
            main_world->load("levels/world1/info");
          }
          current_world = main_world.get();
          update_load_game_menu();
          Menu::push_current(load_game_menu.get());
          break;
        case MNID_LEVELS_CONTRIB:
          // Contrib Menu
          generate_contrib_menu();
          Menu::push_current(contrib_menu.get());
          break;
        case MNID_CREDITS:
          main_loop->push_screen(new TextScroller("credits.txt"),
                                 new FadeOut(0.5));
          break;
        case MNID_QUITMAINMENU:
          main_loop->quit(new FadeOut(0.25));
          break;
      }
    } else if(menu == load_game_menu.get()) {
      /*
      if(event.key.keysym.sym == SDLK_DELETE) {
        int slot = menu->get_active_item_id();
        std::stringstream stream;
        stream << slot;
        std::string str = _("Are you sure you want to delete slot") + stream.str() + "?";
        
        if(confirm_dialog(bkg_title, str.c_str())) {
          str = "save/slot" + stream.str() + ".stsg";
          log_debug << "Removing: " << str << std::endl;
          PHYSFS_delete(str.c_str());
        }

        update_load_save_game_menu(load_game_menu);
        Menu::set_current(main_menu.get());
      }*/
      process_load_game_menu();
    } else if(menu == contrib_menu.get()) {
      check_levels_contrib_menu();
    } else if (menu == contrib_world_menu.get()) {
      check_contrib_world_menu();
    }
  }

  // reopen menu of user closed it (so that the app doesn't close when user
  // accidently hit ESC)
  if(Menu::current() == 0) {
    Menu::set_current(main_menu.get());
  }
}

std::string
TitleScreen::get_slotinfo(int slot)
{
  std::string tmp;
  std::string title;

  std::string basename = current_world->get_basedir();
  basename = basename.substr(0, basename.length()-1);
  std::string worlddirname = FileSystem::basename(basename);
  std::ostringstream stream;
  stream << "save/" << worlddirname << "_" << slot << ".stsg";
  std::string slotfile = stream.str();

  try {
    lisp::Parser parser;
    std::auto_ptr<lisp::Lisp> root (parser.parse(slotfile));

    const lisp::Lisp* savegame = root->get_lisp("supertux-savegame");
    if(!savegame)
      throw std::runtime_error("file is not a supertux-savegame.");

    savegame->get("title", title);
  } catch(std::exception& e) {
    std::ostringstream slottitle;
    slottitle << _("Slot") << " " << slot << " - " << _("Free");
    return slottitle.str();
  }

  std::ostringstream slottitle;
  slottitle << _("Slot") << " " << slot << " - " << title;
  return slottitle.str();
}

bool
TitleScreen::process_load_game_menu()
{
  int slot = load_game_menu->check();

  if(slot == -1)
    return false;

  if(load_game_menu->get_item_by_id(slot).kind != MN_ACTION)
    return false;

  std::string basename = current_world->get_basedir();
  basename = basename.substr(0, basename.length()-1);
  std::string worlddirname = FileSystem::basename(basename);
  std::stringstream stream;
  stream << "save/" << worlddirname << "_" << slot << ".stsg";
  std::string slotfile = stream.str();

  try {
    current_world->set_savegame_filename(slotfile);
    current_world->run();
  } catch(std::exception& e) {
    log_fatal << "Couldn't start world: " << e.what() << std::endl;
  }

  return true;
}

