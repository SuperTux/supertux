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
#include "high_scores.h"
#include "gui/menu.h"
#include "special/timer.h"
#include "special/frame_rate.h"
#include "app/setup.h"
#include "level.h"
#include "level_subset.h"
#include "gameloop.h"
#include "worldmap.h"
#include "leveleditor.h"
#include "scene.h"
#include "player.h"
#include "tile.h"
#include "sector.h"
#include "tilemap.h"
#include "resources.h"
#include "special/base.h"
#include "app/gettext.h"
#include "misc.h"
#include "camera.h"

static Surface* bkg_title;
static Surface* logo;
static Surface* img_choose_subset;

static bool walking;
static Timer random_timer;

static int frame;

static GameSession* titlesession;

static std::vector<LevelSubset*> contrib_subsets;
static LevelSubset* current_contrib_subset = 0;
static int first_level_index;

static std::set<std::string> worldmap_list;

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
  /** Generating contrib levels list by making use of Level Subset */
  std::set<std::string> level_subsets = FileSystem::dsubdirs("/levels", "info");

  free_contrib_menu();

  contrib_menu->additem(MN_LABEL,_("Contrib Levels"),0,0);
  contrib_menu->additem(MN_HL,"",0,0);
  int i = 0;

  for(std::set<std::string>::iterator it = worldmap_list.begin(); it != worldmap_list.end(); ++it)
    {
    WorldMapNS::WorldMap worldmap;
    worldmap.loadmap((*it).c_str());
    contrib_menu->additem(MN_ACTION, worldmap.get_world_title(),0,0, i);
    ++i;
    }

  contrib_menu->additem(MN_HL,"",0,0);

  first_level_index = i;
  for (std::set<std::string>::iterator it = level_subsets.begin(); it != level_subsets.end(); ++it)
    {
      LevelSubset* subset = new LevelSubset();
      subset->load(*it);
      if(subset->hide_from_contribs)
        {
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

void check_levels_contrib_menu()
{
  static int current_subset = -1;

  int index = contrib_menu->check();
  if (index == -1)
    return;

  if((unsigned)index < worldmap_list.size())
    {
    WorldMapNS::WorldMap worldmap;
    std::set<std::string>::iterator it = worldmap_list.begin();
    for(int i = index; i > 0; --i)
    ++it;

    std::string map_filename = *it;

    // some fading
    fadeout(256);
    DrawingContext context;
      context.draw_text(white_text, "Loading...",
                        Vector(screen->w/2, screen->h/2), CENTER_ALLIGN, LAYER_FOREGROUND1);
      context.do_drawing();

    worldmap.set_map_filename(map_filename);

    // hack to erase the extension
    unsigned int ext_pos = it->find_last_of(".");
    if(ext_pos != std::string::npos)
      map_filename.erase(ext_pos, map_filename.size() - ext_pos);

    // TODO: slots should be available for contrib maps
    worldmap.loadgame(st_save_dir + "/" + map_filename + "-slot1.stsg");

    worldmap.display();  // run the map

    Menu::set_current(main_menu);
    resume_demo();
    }
  else if (index < (int)contrib_subsets.size() + first_level_index)
    {
    index -= first_level_index;
    if (current_subset != index)
      {
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
        std::string level_title = "<no title>";

        std::string filename = subset.get_level_filename(i);
        std::string filepath;
        filepath = st_dir + "/levels/" + filename;
        if (access(filepath.c_str(), R_OK) != 0)
        {
          filepath = datadir + "/levels/" + filename;
          if (access(filepath.c_str(), R_OK) != 0)
          {
            std::cerr << "Error: Level: couldn't find level: " << filename << std::endl;
            continue;
          }
        }
        
        LispReader* reader = LispReader::load(filepath, "supertux-level");
        if(!reader)
          {
          std::cerr << "Error: Could not open level file. Ignoring...\n";
          continue;
          }

        reader->read_string("name", level_title, true);
        delete reader;

        contrib_subset_menu->additem(MN_ACTION, level_title, 0, 0, i);
        }

      contrib_subset_menu->additem(MN_HL,"",0,0);      
      contrib_subset_menu->additem(MN_BACK, _("Back"), 0, 0);

      titlesession->get_current_sector()->activate();
      titlesession->set_current();
      }
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

void draw_demo(double frame_ratio)
{
  Sector* world  = titlesession->get_current_sector();
  Player* tux = world->player;

  world->play_music(LEVEL_MUSIC);
  
  global_frame_counter++;
  tux->key_event((SDLKey) keymap.right,DOWN);
  
  if(random_timer.check())
    {
      if(walking)
        tux->key_event((SDLKey) keymap.jump,UP);
      else
        tux->key_event((SDLKey) keymap.jump,DOWN);
    }
  else
    {
      random_timer.start(rand() % 3000 + 3000);
      walking = !walking;
    }

  // Wrap around at the end of the level back to the beginnig
  if(world->solids->get_width() * 32 - 320 < tux->base.x)
    {
      tux->level_begin();
      world->camera->reset(Vector(tux->base.x, tux->base.y));
    }

  tux->can_jump = true;
  float last_tux_x_pos = tux->base.x;
  world->action(frame_ratio);
  

  // disabled for now, since with the new jump code we easily get deadlocks
  // Jump if tux stays in the same position for one loop, ie. if he is
  // stuck behind a wall
  if (last_tux_x_pos == tux->base.x)
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

  random_timer.init(true);
  Ticks::pause_init();

  titlesession = new GameSession("misc/menu.stl", ST_GL_DEMO_GAME);

  /* Load images: */
  bkg_title = new Surface(datadir + "/images/background/arctis.jpg", false);
  logo = new Surface(datadir + "/images/title/logo.png", true);
  img_choose_subset = new Surface(datadir + "/images/status/choose-level-subset.png", true);

  /* Generating contrib maps by only using a string_list */
  worldmap_list = FileSystem::dfiles("levels/worldmap", "", "icyisland.stwm");

  titlesession->get_current_sector()->activate();
  titlesession->set_current();

  /* --- Main title loop: --- */
  frame = 0;

  frame_rate.set_frame_limit(false);
  
  random_timer.start(rand() % 2000 + 2000);

  Menu::set_current(main_menu);
  DrawingContext& context = *titlesession->context;
  while (Menu::current())
    {
      // if we spent to much time on a menu entry
      frame_rate.smooth_hanger();
    
      // Calculate the movement-factor
      double frame_ratio = frame_rate.get();
      
      if(frame_ratio > 1.5) /* Quick hack to correct the unprecise CPU clocks a little bit. */
        frame_ratio = 1.5 + (frame_ratio - 1.5) * 0.85;
      /* Lower the frame_ratio that Tux doesn't jump to hectically throught the demo. */
      frame_ratio /= 2;

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
      draw_demo(frame_ratio);
      
      
      if (Menu::current() == main_menu)
        context.draw_surface(logo, Vector(screen->w/2 - logo->w/2, 30),
            LAYER_FOREGROUND1+1);

      context.draw_text(white_small_text, " SuperTux " VERSION "\n", Vector(0, screen->h - 70), LEFT_ALLIGN, LAYER_FOREGROUND1);
      context.draw_text(white_small_text,
        _("Copyright (c) 2003 SuperTux Devel Team\n"
          "This game comes with ABSOLUTELY NO WARRANTY. This is free software, and you\n"
          "are welcome to redistribute it under certain conditions; see the file COPYING\n"
          "for details.\n"), Vector(0, screen->h - 70 + white_small_text->get_height()), LEFT_ALLIGN, LAYER_FOREGROUND1);

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
                  display_text_file("CREDITS", SCROLL_SPEED_CREDITS, white_big_text , white_text, white_small_text, blue_text );
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
      SDL_Delay(25);
    }
  /* Free surfaces: */

  free_contrib_menu();
  worldmap_list.clear();
  delete titlesession;
  delete bkg_title;
  delete logo;
  delete img_choose_subset;
}


// EOF //

