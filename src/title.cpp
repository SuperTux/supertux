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

#include "defines.h"
#include "app/globals.h"
#include "title.h"
#include "video/screen.h"
#include "video/surface.h"
#include "gui/menu.h"
#include "timer.h"
#include "special/frame_rate.h"
#include "app/setup.h"
#include "lisp/lisp.h"
#include "lisp/parser.h"
#include "level.h"
#include "level_subset.h"
#include "gameloop.h"
#include "worldmap.h"
#include "leveleditor.h"
#include "scene.h"
#include "tile.h"
#include "sector.h"
#include "object/tilemap.h"
#include "object/camera.h"
#include "object/player.h"
#include "resources.h"
#include "app/gettext.h"
#include "misc.h"

static Surface* bkg_title;
static Surface* logo;
static Surface* img_choose_subset;

static bool walking;
static Timer2 random_timer;

static int frame;

static GameSession* titlesession;

static std::vector<LevelSubset*> contrib_subsets;
static LevelSubset* current_contrib_subset = 0;

static FrameRate frame_rate(100);  

/* If the demo was stopped - because game started, level
   editor was excuted, etc - call this when you get back
   to the title code.
 */
void resume_demo()
{
  // FIXME: shouldn't be needed if GameSession
  // didn't relay on global variables
  titlesession->get_current_sector()->activate();
  titlesession->set_current();

  frame_rate.update();
}

void update_load_save_game_menu(Menu* pmenu)
{
  for(int i = 2; i < 7; ++i)
    {
      // FIXME: Insert a real savegame struct/class here instead of
      // doing string vodoo
      std::string tmp = slotinfo(i - 1);
      pmenu->item[i].kind = MN_ACTION;
      pmenu->item[i].change_text(tmp.c_str());
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

  contrib_menu->additem(MN_LABEL,_("Contrib Levels"),0,0);
  contrib_menu->additem(MN_HL,"",0,0);
  
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
      contrib_menu->additem(MN_GOTO, subset->title, 0, contrib_subset_menu, i);
      contrib_subsets.push_back(subset);
      ++i;
    }

  contrib_menu->additem(MN_HL,"",0,0);
  contrib_menu->additem(MN_BACK,_("Back"),0,0);

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
        Vector(screen->w/2, screen->h/2), CENTER_ALLIGN, LAYER_FOREGROUND1);
    context.do_drawing();

    // TODO: slots should be available for contrib maps
    worldmap.loadgame(st_save_dir + "/" + subset.name + "-slot1.stsg");

    worldmap.display();  // run the map

    Menu::set_current(main_menu);
    resume_demo();
  } else if (current_subset != index) {
    current_subset = index;
    // FIXME: This shouln't be busy looping
    LevelSubset& subset = * (contrib_subsets[index]);

    current_contrib_subset = &subset;

    contrib_subset_menu->clear();

    contrib_subset_menu->additem(MN_LABEL, subset.title, 0,0);
    contrib_subset_menu->additem(MN_HL,"",0,0);

    for (int i = 0; i < subset.get_num_levels(); ++i)
    {
      /** get level's title */
      std::string filename = subset.get_level_filename(i);
      std::string title = get_level_name(filename);
      contrib_subset_menu->additem(MN_ACTION, title, 0, 0, i);
    }

    contrib_subset_menu->additem(MN_HL,"",0,0);      
    contrib_subset_menu->additem(MN_BACK, _("Back"), 0, 0);

    titlesession->get_current_sector()->activate();
    titlesession->set_current();
  }
}

void check_contrib_subset_menu()
{
  int index = contrib_subset_menu->check();
  if (index != -1)
    {
      if (contrib_subset_menu->get_item_by_id(index).kind == MN_ACTION)
        {
          std::cout << "Starting level: " << index << std::endl;
          
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
  Sector* world  = titlesession->get_current_sector();
  Player* tux = world->player;

  world->play_music(LEVEL_MUSIC);
  
  tux->key_event((SDLKey) keymap.right,DOWN);
  
  if(random_timer.check()) {
    random_timer.start(float(rand() % 3000 + 3000) / 1000.);
    walking = !walking;
  } else {
      if(walking)
        tux->key_event((SDLKey) keymap.jump,UP);
      else
        tux->key_event((SDLKey) keymap.jump,DOWN);
  }

  // Wrap around at the end of the level back to the beginnig
  if(world->solids->get_width() * 32 - 320 < tux->get_pos().x)
    {
      tux->level_begin();
      world->camera->reset(tux->get_pos());
    }

  tux->can_jump = true;
  float last_tux_x_pos = tux->get_pos().x;
  world->action(elapsed_time);
  

  // disabled for now, since with the new jump code we easily get deadlocks
  // Jump if tux stays in the same position for one loop, ie. if he is
  // stuck behind a wall
  if (last_tux_x_pos == tux->get_pos().x)
    {
      walking = false;
    }

  world->draw(*titlesession->context);
}

/* --- TITLE SCREEN --- */
void title(void)
{
  walking = true;
  LevelEditor* leveleditor;
  MusicRef credits_music;

  Ticks::pause_init();

  titlesession = new GameSession(get_resource_filename("levels/misc/menu.stl"),
      ST_GL_DEMO_GAME);

  /* Load images: */
  bkg_title = new Surface(datadir + "/images/background/arctis.jpg", false);
  logo = new Surface(datadir + "/images/title/logo.png", true);
  img_choose_subset = new Surface(datadir + "/images/status/choose-level-subset.png", true);

  titlesession->get_current_sector()->activate();
  titlesession->set_current();

  /* --- Main title loop: --- */
  frame = 0;

  random_timer.start(float(rand() % 2000 + 2000) / 1000.0);

  Uint32 lastticks = SDL_GetTicks();
  
  Menu::set_current(main_menu);
  DrawingContext& context = *titlesession->context;
  while (Menu::current())
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
      while (SDL_PollEvent(&event))
        {
          if (Menu::current())
            {
              Menu::current()->event(event);
            }
         // FIXME: QUIT signal should be handled more generic, not locally
          if (event.type == SDL_QUIT)
            Menu::set_current(0);
        }
  
      /* Draw the background: */
      draw_demo(elapsed_time);
      
      
      if (Menu::current() == main_menu)
        context.draw_surface(logo, Vector(screen->w/2 - logo->w/2, 30),
            LAYER_FOREGROUND1+1);

      context.draw_text(white_small_text, " SuperTux " PACKAGE_VERSION "\n",
              Vector(0, screen->h - 70), LEFT_ALLIGN, LAYER_FOREGROUND1);
      context.draw_text(white_small_text,
        _(
"Copyright (c) 2003 SuperTux Devel Team\n"
"This game comes with ABSOLUTELY NO WARRANTY. This is free software, and you\n"
"are welcome to redistribute it under certain conditions; see the file COPYING\n"
"for details.\n"
        ),
        Vector(0, screen->h - 70 + white_small_text->get_height()),
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
                  puts("Entering contrib menu");
                  generate_contrib_menu();
                  break;
                case MNID_LEVELEDITOR:
                  leveleditor = new LevelEditor();
                  leveleditor->run();
                  delete leveleditor;
                  Menu::set_current(main_menu);
                  resume_demo();
                  break;
                case MNID_CREDITS:
                  fadeout(500);
                  credits_music = SoundManager::get()->load_music(datadir + "/music/credits.ogg");
                  SoundManager::get()->play_music(credits_music);
                  display_text_file("credits.txt", SCROLL_SPEED_CREDITS, white_big_text , white_text, white_small_text, blue_text );
                  fadeout(500);
                  Menu::set_current(main_menu);
                  break;
                case MNID_QUITMAINMENU:
                  Menu::set_current(0);
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
                  str = st_save_dir + "/slot" + stream.str() + ".stsg";
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

      mouse_cursor->draw(context);
     
      context.do_drawing();

      frame_rate.update();

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

