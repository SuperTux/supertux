//  $Id$
// 
//  SuperTux
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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
#include "video/screen.hpp"
#include "video/surface.hpp"
#include "audio/sound_manager.hpp"
#include "gui/menu.hpp"
#include "timer.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "level.hpp"
#include "level_subset.hpp"
#include "game_session.hpp"
#include "worldmap.hpp"
#include "player_status.hpp"
#include "tile.hpp"
#include "sector.hpp"
#include "object/tilemap.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "resources.hpp"
#include "gettext.hpp"
#include "misc.hpp"
#include "textscroller.hpp"
#include "file_system.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "control/codecontroller.hpp"
#include "main.hpp"
#include "exceptions.hpp"

static Surface* bkg_title;
static Surface* logo;
//static Surface* img_choose_subset;

static bool walking;
static Timer random_timer;

static int frame;

static GameSession* titlesession;
static CodeController* controller;

static std::vector<LevelSubset*> contrib_subsets;
static LevelSubset* current_contrib_subset = 0;
static int current_subset = -1;

/* If the demo was stopped - because game started, level
   editor was excuted, etc - call this when you get back
   to the title code.
 */
void resume_demo()
{
  player_status->reset();
  titlesession->get_current_sector()->activate("main");
  titlesession->set_current();

  //frame_rate.update();
}

void update_load_save_game_menu(Menu* menu)
{
  printf("update loadsavemenu.\n");
  for(int i = 1; i < 6; ++i) {
    MenuItem& item = menu->get_item_by_id(i);
    item.kind = MN_ACTION;
    item.change_text(slotinfo(i));
  }
}

void free_contrib_menu()
{
  for(std::vector<LevelSubset*>::iterator i = contrib_subsets.begin();
      i != contrib_subsets.end(); ++i)
    delete *i;

  contrib_subsets.clear();
  contrib_menu->clear();
  current_contrib_subset = 0;
  current_subset = -1;
}

void generate_contrib_menu()
{
  /** Generating contrib levels list by making use of Level Subset  */
  std::vector<std::string> level_subsets; 
  char** files = PHYSFS_enumerateFiles("levels/");
  for(const char* const* filename = files; *filename != 0; ++filename) {
    std::string filepath = std::string("levels/") + *filename;
    if(PHYSFS_isDirectory(filepath.c_str()))
      level_subsets.push_back(filepath);
  }
  PHYSFS_freeList(files);

  free_contrib_menu();

  contrib_menu->add_label(_("Contrib Levels"));
  contrib_menu->add_hl();
  
  int i = 0;
  for (std::vector<std::string>::iterator it = level_subsets.begin();
      it != level_subsets.end(); ++it) {
    try {
      std::auto_ptr<LevelSubset> subset (new LevelSubset());
      subset->load(*it);
      if(subset->hide_from_contribs) {
        continue;
      }
      contrib_menu->add_submenu(subset->title, contrib_subset_menu, i++);
      contrib_subsets.push_back(subset.release());
    } catch(std::exception& e) {
#ifdef DEBUG
      std::cerr << "Couldn't parse levelset info for '"
        << *it << "': " << e.what() << "\n";
#endif
    }
  }

  contrib_menu->add_hl();
  contrib_menu->add_back(_("Back"));
}

std::string get_level_name(const std::string& filename)
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
    std::cerr << "Problem getting name of '" << filename << "'.\n";
    return "";
  }
}

void check_levels_contrib_menu()
{
  int index = contrib_menu->check();
  if (index == -1)
    return;

  LevelSubset& subset = * (contrib_subsets[index]);
  
  if(subset.has_worldmap) {
    WorldMapNS::WorldMap worldmap;
    worldmap.set_map_filename(subset.get_worldmap_filename());
    sound_manager->stop_music();

    // some fading
    fadeout(256);
    DrawingContext context;
    context.draw_text(white_text, "Loading...",
        Vector(SCREEN_WIDTH/2, SCREEN_HEIGHT/2), CENTER_ALLIGN, LAYER_FOREGROUND1);
    context.do_drawing();

    // TODO: slots should be available for contrib maps
    worldmap.loadgame("save/" + subset.name + "-slot1.stsg");
    worldmap.display();  // run the map

    Menu::set_current(main_menu);
    resume_demo();
  } else if (current_subset != index) {
    current_subset = index;
    LevelSubset& subset = * (contrib_subsets[index]);

    current_contrib_subset = &subset;

    contrib_subset_menu->clear();

    contrib_subset_menu->add_label(subset.title);
    contrib_subset_menu->add_hl();

    for (int i = 0; i < subset.get_num_levels(); ++i)
    {
      /** get level's title */
      std::string filename = subset.get_level_filename(i);
      std::string title = get_level_name(filename);
      contrib_subset_menu->add_entry(i, title);
    }

    contrib_subset_menu->add_hl();
    contrib_subset_menu->add_back(_("Back"));

    titlesession->get_current_sector()->activate("main");
    titlesession->set_current();
  }
}

void check_contrib_subset_menu()
{
  int index = contrib_subset_menu->check();
  if (index != -1) {
    if (contrib_subset_menu->get_item_by_id(index).kind == MN_ACTION) {
      sound_manager->stop_music();
      GameSession session(
          current_contrib_subset->get_level_filename(index), ST_GL_PLAY);
      session.run();
      player_status->reset();
      Menu::set_current(main_menu);
      resume_demo();
    }
  }  
}

void draw_demo(float elapsed_time)
{
  static float last_tux_x_pos = -1;
  static float last_tux_y_pos = -1;
  Sector* sector  = titlesession->get_current_sector();
  Player* tux = sector->player;

  sector->play_music(LEVEL_MUSIC);

  controller->update();
  controller->press(Controller::RIGHT);
  
  if(random_timer.check() || 
      (walking && fabsf(last_tux_x_pos - tux->get_pos().x)) < .1) {
    walking = false;
  } else {
      if(!walking && fabsf(tux->get_pos().y - last_tux_y_pos) < .1) {
        random_timer.start(float(rand() % 3000 + 3000) / 1000.);
        walking = true;
      }
  }
  if(!walking)
    controller->press(Controller::JUMP);
  last_tux_x_pos = tux->get_pos().x;
  last_tux_y_pos = tux->get_pos().y;

  // Wrap around at the end of the level back to the beginnig
  if(sector->solids->get_width() * 32 - 320 < tux->get_pos().x) {
    sector->activate("main");
    sector->camera->reset(tux->get_pos());
  }

  sector->update(elapsed_time);
  sector->draw(*titlesession->context);
}

/* --- TITLE SCREEN --- */
void title()
{
  walking = true;
  //LevelEditor* leveleditor;
  controller = new CodeController();

  titlesession = new GameSession("levels/misc/menu.stl", ST_GL_DEMO_GAME);

  /* Load images: */
  bkg_title = new Surface("images/background/arctis.jpg");
  logo = new Surface("images/engine/menu/logo.png");
  //img_choose_subset = new Surface("images/status/choose-level-subset.png");

  titlesession->get_current_sector()->activate("main");
  titlesession->set_current();

  Player* player = titlesession->get_current_sector()->player;
  player->set_controller(controller);

  /* --- Main title loop: --- */
  frame = 0;

  random_timer.start(float(rand() % 2000 + 2000) / 1000.0);

  Uint32 lastticks = SDL_GetTicks();
  
  Menu::set_current(main_menu);
  DrawingContext& context = *titlesession->context;
  bool running = true;
  while (running)
    {
      // Calculate the movement-factor
      Uint32 ticks = SDL_GetTicks();
      float elapsed_time = float(ticks - lastticks) / 1000.;
      game_time += elapsed_time;
      lastticks = ticks;
      // 40fps is minimum
      if(elapsed_time > .04)
        elapsed_time = .04;
      
      /* Lower the speed so that Tux doesn't jump too hectically throught
         the demo. */
      elapsed_time /= 2;

      SDL_Event event;
      main_controller->update();
      while (SDL_PollEvent(&event)) {
        if (Menu::current()) {
          Menu::current()->event(event);
        }
        main_controller->process_event(event);
        if (event.type == SDL_QUIT)
          throw graceful_shutdown();
      }
  
      /* Draw the background: */
      draw_demo(elapsed_time);

      if (Menu::current() == main_menu)
        context.draw_surface(logo, Vector(SCREEN_WIDTH/2 - logo->get_width()/2, 30),
            LAYER_FOREGROUND1+1);

      context.draw_text(white_small_text, " SuperTux " PACKAGE_VERSION "\n",
              Vector(0, SCREEN_HEIGHT - 50), LEFT_ALLIGN, LAYER_FOREGROUND1);
      context.draw_text(white_small_text,
        _(
"Copyright (c) 2005 SuperTux Devel Team\n"
"This game comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to\n"
"redistribute it under certain conditions; see the file COPYING for details.\n"
        ),
        Vector(0, SCREEN_HEIGHT - 50 + white_small_text->get_height() + 5),
        LEFT_ALLIGN, LAYER_FOREGROUND1);

      /* Don't draw menu, if quit is true */
      Menu* menu = Menu::current();
      if(menu)
        {
          menu->draw(context);
          menu->update();
  	  
          if(menu == main_menu)
            {
              switch (main_menu->check())
                {
                case MNID_STARTGAME:
                  // Start Game, ie. goto the slots menu
                  update_load_save_game_menu(load_game_menu);
                  break;
                case MNID_LEVELS_CONTRIB:
                  // Contrib Menu
                  generate_contrib_menu();
                  break;
#if 0
                case MNID_LEVELEDITOR: {
                  LevelEdtiro* leveleditor = new LevelEditor();
                  leveleditor->run();
                  delete leveleditor;
                  Menu::set_current(main_menu);
                  resume_demo();
                  break;
                }
#endif
                case MNID_CREDITS:
                  sound_manager->stop_music();
                  fadeout(500);
                  sound_manager->play_music("music/credits.ogg");
                  display_text_file("credits.txt");
                  sound_manager->stop_music();
                  fadeout(500);
                  Menu::set_current(main_menu);
                  break;
                case MNID_QUITMAINMENU:
                  running = false;
                  break;
                }
            }
          else if(menu == options_menu)
            {
              process_options_menu();
            }
          else if(menu == load_game_menu)
            {
              if(event.key.keysym.sym == SDLK_DELETE)
                {
                int slot = menu->get_active_item_id();
                std::stringstream stream;
                stream << slot;
                std::string str = _("Are you sure you want to delete slot") + stream.str() + "?";
                
                if(confirm_dialog(bkg_title, str.c_str())) {
                  str = "save/slot" + stream.str() + ".stsg";
                  printf("Removing: %s\n",str.c_str());
                  PHYSFS_delete(str.c_str());
                }

                update_load_save_game_menu(load_game_menu);
                Menu::set_current(main_menu);
                resume_demo();
                }
              else if (process_load_game_menu())
                {
                  resume_demo();
                }
            }
          else if(menu == contrib_menu)
            {
              check_levels_contrib_menu();
            }
          else if (menu == contrib_subset_menu)
            {
              check_contrib_subset_menu();
            }
        }

      // reopen menu of user closed it (so that the app doesn't close when user
      // accidently hit ESC)
      if(Menu::current() == 0) {
        Menu::set_current(main_menu);
      }

      context.do_drawing();
      sound_manager->update();

      //frame_rate.update();

      /* Pause: */
      frame++;
    }
  /* Free surfaces: */

  free_contrib_menu();
  delete titlesession;
  delete bkg_title;
  delete logo;
  //delete img_choose_subset;
}
