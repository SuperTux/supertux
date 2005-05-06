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

#ifndef WIN32
#include <sys/types.h>
#include <ctype.h>
#endif

#include "title.h"
#include "video/screen.h"
#include "video/surface.h"
#include "gui/menu.h"
#include "timer.h"
#include "lisp/lisp.h"
#include "lisp/parser.h"
#include "level.h"
#include "level_subset.h"
#include "game_session.h"
#include "worldmap.h"
#include "leveleditor.h"
#include "player_status.h"
#include "tile.h"
#include "sector.h"
#include "object/tilemap.h"
#include "object/camera.h"
#include "object/player.h"
#include "resources.h"
#include "gettext.h"
#include "misc.h"
#include "textscroller.h"
#include "file_system.h"
#include "control/joystickkeyboardcontroller.h"
#include "control/codecontroller.h"
#include "main.h"

static Surface* bkg_title;
static Surface* logo;
static Surface* img_choose_subset;

static bool walking;
static Timer random_timer;

static int frame;

static GameSession* titlesession;
static CodeController* controller;

static std::vector<LevelSubset*> contrib_subsets;
static LevelSubset* current_contrib_subset = 0;

/* If the demo was stopped - because game started, level
   editor was excuted, etc - call this when you get back
   to the title code.
 */
void resume_demo()
{
  player_status.reset();
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
}

void generate_contrib_menu()
{
  /** Generating contrib levels list by making use of Level Subset  */
  std::set<std::string> level_subsets = FileSystem::dsubdirs("/levels", "info");

  free_contrib_menu();

  contrib_menu->add_label(_("Contrib Levels"));
  contrib_menu->add_hl();
  
  int i = 0;
  for (std::set<std::string>::iterator it = level_subsets.begin();
      it != level_subsets.end(); ++it)
    {
      LevelSubset* subset = new LevelSubset();
      subset->load(*it);
      if(subset->hide_from_contribs) {
        delete subset;
        continue;
      }
      contrib_menu->add_submenu(subset->title, contrib_subset_menu, i);
      contrib_subsets.push_back(subset);
      ++i;
    }

  contrib_menu->add_hl();
  contrib_menu->add_back(_("Back"));
  
  level_subsets.clear();
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
  static int current_subset = -1;

  int index = contrib_menu->check();
  if (index == -1)
    return;

  LevelSubset& subset = * (contrib_subsets[index]);
  
  if(subset.has_worldmap) {
    WorldMapNS::WorldMap worldmap;
    worldmap.set_map_filename(subset.get_worldmap_filename());

    // some fading
    fadeout(256);
    DrawingContext context;
    context.draw_text(white_text, "Loading...",
        Vector(SCREEN_WIDTH/2, SCREEN_HEIGHT/2), CENTER_ALLIGN, LAYER_FOREGROUND1);
    context.do_drawing();

    // TODO: slots should be available for contrib maps
    worldmap.loadgame(user_dir + "/save/" + subset.name + "-slot1.stsg");

    worldmap.display();  // run the map

    Menu::set_current(main_menu);
    resume_demo();
  } else if (current_subset != index) {
    current_subset = index;
    // FIXME: This shouln't be busy looping
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
      GameSession session(
          current_contrib_subset->get_level_filename(index), ST_GL_PLAY);
      session.run();
      player_status.reset();
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

  sector->action(elapsed_time);
  sector->draw(*titlesession->context);
}

/* --- TITLE SCREEN --- */
void title()
{
  walking = true;
  //LevelEditor* leveleditor;
  MusicRef credits_music;
  controller = new CodeController();

  titlesession = new GameSession(get_resource_filename("levels/misc/menu.stl"),
      ST_GL_DEMO_GAME);

  /* Load images: */
  bkg_title = new Surface(datadir + "/images/background/arctis.jpg", false);
  logo = new Surface(datadir + "/images/title/logo.png", true);
  img_choose_subset = new Surface(datadir + "/images/status/choose-level-subset.png", true);

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
      global_time += elapsed_time;
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
          throw std::runtime_error("Received window close");
      }
  
      /* Draw the background: */
      draw_demo(elapsed_time);

      if (Menu::current() == main_menu)
        context.draw_surface(logo, Vector(SCREEN_WIDTH/2 - logo->w/2, 30),
            LAYER_FOREGROUND1+1);

      context.draw_text(white_small_text, " SuperTux " PACKAGE_VERSION "\n",
              Vector(0, SCREEN_HEIGHT - 50), LEFT_ALLIGN, LAYER_FOREGROUND1);
      context.draw_text(white_small_text,
        _(
"Copyright (c) 2003 SuperTux Devel Team\n"
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
          menu->action();
  	  
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
                  fadeout(500);
                  credits_music = sound_manager->load_music(
                    get_resource_filename("/music/credits.ogg"));
                  sound_manager->play_music(credits_music);
                  display_text_file("credits.txt");
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
                
                if(confirm_dialog(bkg_title, str.c_str()))
                  {
                  str = user_dir + "/save/slot" + stream.str() + ".stsg";
                  printf("Removing: %s\n",str.c_str());
                  remove(str.c_str());
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

      mouse_cursor->draw(context);
     
      context.do_drawing();

      //frame_rate.update();

      /* Pause: */
      frame++;
    }
  /* Free surfaces: */

  free_contrib_menu();
  delete titlesession;
  delete bkg_title;
  delete logo;
  delete img_choose_subset;
}
