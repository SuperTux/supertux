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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>

#ifndef WIN32
#include <sys/types.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "globals.h"
#include "title.h"
#include "screen.h"
#include "high_scores.h"
#include "menu.h"
#include "texture.h"
#include "timer.h"
#include "setup.h"
#include "level.h"
#include "gameloop.h"
#include "leveleditor.h"
#include "scene.h"
#include "player.h"
#include "math.h"
#include "tile.h"
#include "resources.h"

static Surface* bkg_title;
static Surface* logo;
static Surface* img_choose_subset;

static bool walking;
static Timer random_timer;

static int frame;
static unsigned int last_update_time;
static unsigned int update_time;

void display_credits();

std::vector<st_subset> contrib_subsets;
std::string current_contrib_subset;

void generate_contrib_menu()
{
  string_list_type level_subsets = dsubdirs("/levels", "info");

  contrib_menu->clear();
  contrib_menu->additem(MN_LABEL,"Contrib Levels",0,0);
  contrib_menu->additem(MN_HL,"",0,0);

  for (int i = 0; i < level_subsets.num_items; ++i)
    {
      st_subset subset;
      subset.load(level_subsets.item[i]);
      contrib_menu->additem(MN_GOTO, subset.title.c_str(), i, contrib_subset_menu);
      contrib_subsets.push_back(subset);
    }

  contrib_menu->additem(MN_HL,"",0,0);
  contrib_menu->additem(MN_BACK,"Back",0,0);

  string_list_free(&level_subsets);
}

void check_contrib_menu()
{
  static int current_subset = -1;

  int index = contrib_menu->check();
  if (index != -1)
    {
      index -= 2; // FIXME: Hack
      if (index >= 0 && index <= int(contrib_subsets.size()))
        {
          if (current_subset != index)
            {
              current_subset = index;
              // FIXME: This shouln't be busy looping
              st_subset& subset = contrib_subsets[index];
          
              current_contrib_subset = subset.name;

              std::cout << "Updating the contrib subset menu..." << subset.levels << std::endl;
      
              contrib_subset_menu->clear();

              contrib_subset_menu->additem(MN_LABEL, subset.title, 0,0);
              contrib_subset_menu->additem(MN_HL,"",0,0);
              for (int i = 1; i <= subset.levels; ++i)
                {
                  Level level;
                  level.load(subset.name, i);
                  contrib_subset_menu->additem(MN_ACTION, level.name, 0, 0);
                }
              contrib_subset_menu->additem(MN_HL,"",0,0);      
              contrib_subset_menu->additem(MN_BACK, "Back", 0, 0);
            }
        }
      else
        {
          // Back button
        }
    }
}

void check_contrib_subset_menu()
{
  int index = contrib_subset_menu->check();
  if (index != -1)
    {
      if (contrib_subset_menu->get_item(index).kind == MN_ACTION)
        {
          index -= 1; // FIXME: Hack
          std::cout << "Sarting level: " << index << std::endl;
          halt_music();
          GameSession session(current_contrib_subset, index, ST_GL_PLAY);
          session.run();
          Menu::set_current(main_menu);
        }
    }  
}

void draw_background()
{
  /* Draw the title background: */

  bkg_title->draw_bg();
}

void draw_demo(GameSession* session, double frame_ratio)
{
  World::set_current(session->get_world());
  //World* world  = session->get_world();
  Level* plevel = session->get_level();
  Player* tux = session->get_world()->get_tux();

  session->get_world()->play_music(LEVEL_MUSIC);
  
  /* FIXME:
  // update particle systems
  std::vector<ParticleSystem*>::iterator p;
  for(p = particle_systems.begin(); p != particle_systems.end(); ++p)
    {
      (*p)->simulate(frame_ratio);
    }

  // Draw particle systems (background)
  for(p = particle_systems.begin(); p != particle_systems.end(); ++p)
    {
      (*p)->draw(scroll_x, 0, 0);
    }
  */

  // Draw interactive tiles:
  for (int y = 0; y < 15; ++y)
    {
      for (int x = 0; x < 21; ++x)
        {
          Tile::draw(32*x - fmodf(scroll_x, 32), y * 32,
                     plevel->ia_tiles[(int)y][(int)x + (int)(scroll_x / 32)]);
        }
    }

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
  if(plevel->width * 32 - 320 < tux->base.x)
    {
      tux->base.x = tux->base.x - (plevel->width * 32 - 640);
      scroll_x = tux->base.x - 320;
    }

  float last_tux_x_pos = tux->base.x;
  tux->action(frame_ratio);

  // Jump if tux stays in the same position for one loop, ie. if he is
  // stuck behind a wall
  if (last_tux_x_pos == tux->base.x)
    walking = false;

  tux->draw();
}

/* --- TITLE SCREEN --- */
void title(void)
{
  st_subset subset;
  random_timer.init(true);

  walking = true;

  st_pause_ticks_init();

  GameSession session(datadir + "/levels/misc/menu.stl", 0, ST_GL_DEMO_GAME);

  clearscreen(0, 0, 0);
  updatescreen();

  /* Load images: */
  bkg_title = new Surface(datadir + "/images/title/background.jpg", IGNORE_ALPHA);
  logo = new Surface(datadir + "/images/title/logo.png", USE_ALPHA);
  img_choose_subset = new Surface(datadir + "/images/status/choose-level-subset.png", USE_ALPHA);

  /* --- Main title loop: --- */
  frame = 0;

  /* Draw the title background: */
  bkg_title->draw_bg();

  update_time = st_get_ticks();
  random_timer.start(rand() % 2000 + 2000);

  Menu::set_current(main_menu);
  while (Menu::current())
    {
      // Calculate the movement-factor
      double frame_ratio = ((double)(update_time-last_update_time))/((double)FRAME_RATE);
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
      draw_background();
      draw_demo(&session, frame_ratio);
      
      if (Menu::current() == main_menu)
        logo->draw( 160, 30);

      white_small_text->draw(" SuperTux " VERSION "\n"
                             "Copyright (c) 2003 SuperTux Devel Team\n"
                             "This game comes with ABSOLUTELY NO WARRANTY. This is free software, and you\n"
                             "are welcome to redistribute it under certain conditions; see the file COPYING\n"
                             "for details.\n",
                             0, 420, 0);

      /* Don't draw menu, if quit is true */
      Menu* menu = Menu::current();
      if(menu)
        {
          menu->draw();
          menu->action();
        
          if(menu == main_menu)
            {
              switch (main_menu->check())
                {
                case MNID_STARTGAME:
                  // Start Game, ie. goto the slots menu
                  update_load_save_game_menu(load_game_menu);
                  break;
                case MNID_CONTRIB:
                  // Contrib Menu
                  puts("Entering contrib menu");
                  generate_contrib_menu();
                  break;
                case MNID_LEVELEDITOR:
                  halt_music();
                  leveleditor(1);
                  Menu::set_current(main_menu);
                  break;
                case MNID_CREDITS:
                  display_credits();
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
              if (process_load_game_menu())
                {
                  // FIXME: shouldn't be needed if GameSession doesn't relay on global variables
                  // reset tux
                  scroll_x = 0;
                  //titletux.level_begin();
                  update_time = st_get_ticks();
                }
            }
          else if(menu == contrib_menu)
            {
              check_contrib_menu();
            }
          else if (menu == contrib_subset_menu)
            {
              check_contrib_subset_menu();
            }
        }

      mouse_cursor->draw();
      
      flipscreen();

      /* Set the time of the last update and the time of the current update */
      last_update_time = update_time;
      update_time = st_get_ticks();

      /* Pause: */
      frame++;
      SDL_Delay(25);
    }
  /* Free surfaces: */

  delete bkg_title;
  delete logo;
}

#define MAX_VEL 10
#define SPEED   1
#define SCROLL  60
#define ITEMS_SPACE 4

void display_credits()
{
  int done;
  int scroll, speed;
  int y;
  Timer timer;
  int length;
  FILE* fi;
  char temp[1024];
  string_list_type names;
  char filename[1024];
  string_list_init(&names);
  sprintf(filename,"%s/CREDITS", datadir.c_str());
  if((fi = fopen(filename,"r")) != NULL)
    {
      while(fgets(temp, sizeof(temp), fi) != NULL)
        {
          temp[strlen(temp)-1]='\0';
          string_list_add_item(&names,temp);
        }
      fclose(fi);
    }
  else
    {
      string_list_add_item(&names,"Credits were not found!");
      string_list_add_item(&names,"Shame on the guy, who");
      string_list_add_item(&names,"forgot to include them");
      string_list_add_item(&names,"in your SuperTux distribution.");
    }


  timer.init(SDL_GetTicks());
  timer.start(50);

  scroll = 0;
  speed = 2;
  done = 0;

  length = names.num_items;

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  while(done == 0)
    {
      /* in case of input, exit */
      SDL_Event event;
      while(SDL_PollEvent(&event))
        switch(event.type)
          {
          case SDL_KEYDOWN:
            switch(event.key.keysym.sym)
              {
              case SDLK_UP:
                speed -= SPEED;
                break;
              case SDLK_DOWN:
                speed += SPEED;
                break;
              case SDLK_SPACE:
              case SDLK_RETURN:
                if(speed >= 0)
                  scroll += SCROLL;
                break;
              case SDLK_ESCAPE:
                done = 1;
                break;
              default:
                break;
              }
            break;
          case SDL_QUIT:
            done = 1;
            break;
          default:
            break;
          }

      if(speed > MAX_VEL)
        speed = MAX_VEL;
      else if(speed < -MAX_VEL)
        speed = -MAX_VEL;

      /* draw the credits */

      draw_background();

      white_big_text->drawf("- Credits -", 0, screen->h-scroll, A_HMIDDLE, A_TOP, 2);

      y = 0;
      for(int i = 0; i < length; i++)
        {
        switch(names.item[i][0])
          {
          case ' ':
            white_small_text->drawf(names.item[i], 0, 60+screen->h+y-scroll, A_HMIDDLE, A_TOP, 1);
            y += white_small_text->h+ITEMS_SPACE;
            break;
          case '	':
            white_text->drawf(names.item[i], 0, 60+screen->h+y-scroll, A_HMIDDLE, A_TOP, 1);
            y += white_text->h+ITEMS_SPACE;
            break;
          case '-':
            white_big_text->drawf(names.item[i], 0, 60+screen->h+y-scroll, A_HMIDDLE, A_TOP, 3);
            y += white_big_text->h+ITEMS_SPACE;
            break;
          default:
            blue_text->drawf(names.item[i], 0, 60+screen->h+y-scroll, A_HMIDDLE, A_TOP, 1);
            y += blue_text->h+ITEMS_SPACE;
            break;
          }
        }

      flipscreen();

      if(60+screen->h+y-scroll < 0 && 20+60+screen->h+y-scroll < 0)
        done = 1;

      scroll += speed;
      if(scroll < 0)
        scroll = 0;

      SDL_Delay(35);

      if(timer.get_left() < 0)
        {
          frame++;
          timer.start(50);
        }
    }
  string_list_free(&names);

  SDL_EnableKeyRepeat(0, 0);    // disables key repeating
  Menu::set_current(main_menu);
}
