//  $Id$
// 
//  SuperTux
//  Copyright (C) 2003 Ricardo Cruz <rick2@aeiou.pt>
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>
#include "leveleditor.h"

#include "screen.h"
#include "defines.h"
#include "globals.h"
#include "setup.h"
#include "menu.h"
#include "level.h"
#include "gameloop.h"
#include "badguy.h"
#include "scene.h"
#include "button.h"
#include "tile.h"
#include "resources.h"
#include "music_manager.h"

/* definitions to aid development */

/* definitions that affect gameplay */
#define KEY_CURSOR_SPEED 32
#define KEY_CURSOR_FASTSPEED 64

/* when pagedown/up pressed speed:*/
#define PAGE_CURSOR_SPEED 13*32

#define MOUSE_LEFT_MARGIN 80
#define MOUSE_RIGHT_MARGIN (560-32)
/* right_margin should noticed that the cursor is 32 pixels,
   so it should subtract that value */
#define MOUSE_POS_SPEED 20

/* look */
#define SELECT_W 2 // size of the selections lines
#define SELECT_CLR 0, 255, 0, 255  // lines color (R, G, B, A)

/* own declerations */
/* crutial ones (main loop) */
int le_init();
void le_quit();
void le_drawlevel();
void le_drawinterface();
void le_checkevents();
void le_change(float x, float y, int tm, unsigned int c);
void le_testlevel();
void le_showhelp();
void le_set_defaults(void);
void le_activate_bad_guys(void);

void le_highlight_selection();

void apply_level_settings_menu();
void update_subset_settings_menu();
void save_subset_settings_menu();

static Level* le_current_level;

struct LevelEditorWorld
{
  std::vector<BadGuy> bad_guys;
  void arrays_free(void)
  {
    bad_guys.clear();
  }

  void add_bad_guy(float x, float y, BadGuyKind kind)
  {
    bad_guys.push_back(BadGuy(x,y,kind, false /* stay_on_platform */));
  }

  void activate_bad_guys()
  {
    for (std::vector<BadGuyData>::iterator i = le_current_level->badguy_data.begin();
         i != le_current_level->badguy_data.end();
         ++i)
      {
        add_bad_guy(i->x, i->y, i->kind);
      }
  }
};

/* leveleditor internals */
static string_list_type level_subsets;
static bool le_level_changed;  /* if changes, ask for saving, when quiting*/
static int pos_x, cursor_x, cursor_y, fire;
static int le_level;
static LevelEditorWorld le_world;
static LevelSubset le_level_subset;
static int le_show_grid;
static int le_frame;
static Surface* le_selection;
static int done;
static unsigned int le_current_tile;
static bool le_mouse_pressed[2];
static Button* le_save_level_bt;
static Button* le_exit_bt;
static Button* le_test_level_bt;
static Button* le_next_level_bt;
static Button* le_previous_level_bt;
static Button* le_move_right_bt;
static Button* le_move_left_bt;
static Button* le_rubber_bt;
static Button* le_select_mode_one_bt;
static Button* le_select_mode_two_bt;
static Button* le_settings_bt;
static Button* le_tilegroup_bt;
static ButtonPanel* le_tilemap_panel;
static Menu* leveleditor_menu;
static Menu* subset_load_menu;
static Menu* subset_new_menu;
static Menu* subset_settings_menu;
static Menu* level_settings_menu;
static Menu* select_tilegroup_menu;
static Timer select_tilegroup_menu_effect;
static std::map<std::string, ButtonPanel* > tilegroups_map;
static std::string cur_tilegroup;

static square selection;
static int le_selection_mode;
static SDL_Event event;
TileMapType active_tm;

void le_set_defaults()
{
  if(le_current_level != NULL)
    {
      /* Set defaults: */

      if(le_current_level->time_left == 0)
        le_current_level->time_left = 255;
    }
}

int leveleditor(int levelnb)
{
  int last_time, now_time, i;

  le_level = levelnb;
  if(le_init() != 0)
    return 1;

  /* Clear screen: */

  clearscreen(0, 0, 0);
  updatescreen();
  
  music_manager->halt_music();

  while (SDL_PollEvent(&event))
    {}

  while(true)
    {
      last_time = SDL_GetTicks();
      le_frame++;

      le_checkevents();

      if(Menu::current() == select_tilegroup_menu)
        {
          if(select_tilegroup_menu_effect.check())
            {
              select_tilegroup_menu->set_pos(screen->w - 64 + select_tilegroup_menu_effect.get_left(),
                                             82,-0.5,0.5);
            }
          else
            select_tilegroup_menu->set_pos(screen->w - 64,82,-0.5,0.5);
        }

      if(le_current_level != NULL)
        {
          /* making events results to be in order */
          if(pos_x < 0)
            pos_x = 0;
          if(pos_x > (le_current_level->width * 32) - screen->w)
            pos_x = (le_current_level->width * 32) - screen->w;

          /* draw the level */
          le_drawlevel();
        }
      else
        clearscreen(0, 0, 0);

      /* draw editor interface */
      le_drawinterface();

      Menu* menu = Menu::current();
      if(menu)
        {
          menu->draw();
          menu->action();

          if(menu == leveleditor_menu)
            {
              switch (leveleditor_menu->check())
                {
                case MNID_RETURNLEVELEDITOR:
                  Menu::set_current(0);
                  break;
                case MNID_SUBSETSETTINGS:
                  update_subset_settings_menu();
                  break;
                case MNID_QUITLEVELEDITOR:
                  done = 1;
                  break;
                }
            }
          else if(menu == level_settings_menu)
            {
              switch (level_settings_menu->check())
                {
                case MNID_SUBSETSETTINGS:
                  apply_level_settings_menu();
                  Menu::set_current(leveleditor_menu);
                  break;
                  
                default:
                  //show_menu = true;
                  break;
                }
            }
          else if(menu == select_tilegroup_menu)
            {
              int it = -1;
              switch (it = select_tilegroup_menu->check())
                {
                default:
		  if(it != -1)
                    {
                      if(select_tilegroup_menu->item[it].kind == MN_ACTION)
                        cur_tilegroup = select_tilegroup_menu->item[it].text;
		  
                      Menu::set_current(0);
                    }
                  break;
                }
            }
          else if(menu == subset_load_menu)
            {
              switch (i = subset_load_menu->check())
                {
                case 0:
                  break;
                default:
                  if(i != -1)
                    {
                      le_level_subset.load(level_subsets.item[i-2]);
                      leveleditor_menu->item[3].kind = MN_GOTO;
                      le_level = 1;
                      le_world.arrays_free();
                      le_current_level = new Level;
                      if(le_current_level->load(le_level_subset.name.c_str(), le_level) != 0)
                        {
                          le_quit();
                          return 1;
                        }
                      le_set_defaults();
                      le_current_level->load_gfx();
		      le_world.activate_bad_guys();

                      // FIXME:?
                      Menu::set_current(leveleditor_menu);
                    }
                  break;
                }
            }
          else if(menu == subset_new_menu)
            {
              if(subset_new_menu->item[2].input[0] == '\0')
                subset_new_menu->item[3].kind = MN_DEACTIVE;
              else
                {
                  subset_new_menu->item[3].kind = MN_ACTION;

                  switch (i = subset_new_menu->check())
                    {
                    case 3:
                      LevelSubset::create(subset_new_menu->item[2].input);
                      le_level_subset.load(subset_new_menu->item[2].input);
                      leveleditor_menu->item[3].kind = MN_GOTO;
                      le_level = 1;
                      le_world.arrays_free();
                      le_current_level = new Level;
                      if(le_current_level->load(le_level_subset.name.c_str(), le_level) != 0)
                        {
                          le_quit();
                          return 1;
                        }
                      le_set_defaults();
                      le_current_level->load_gfx();
		      le_world.activate_bad_guys();
                      subset_new_menu->item[2].change_input("");
                      // FIXME:? show_menu = true;
                      Menu::set_current(leveleditor_menu);
                      break;
                    }
                }
            }
          else if(menu == subset_settings_menu)
            {
              if(le_level_subset.title.compare(subset_settings_menu->item[2].input) == 0 && le_level_subset.description.compare(subset_settings_menu->item[3].input) == 0  )
                subset_settings_menu->item[5].kind = MN_DEACTIVE;
              else
                subset_settings_menu->item[5].kind = MN_ACTION;

              switch (i = subset_settings_menu->check())
                {
                case 5:
                  save_subset_settings_menu();
                  //FIXME:show_menu = true;
                  Menu::set_current(leveleditor_menu);
                  break;
                }
            }
        }

      mouse_cursor->draw();

      if(done)
        {
          le_quit();
          return 0;
        }

      ++global_frame_counter;
	
      SDL_Delay(25);
      now_time = SDL_GetTicks();
      if (now_time < last_time + FPS)
        SDL_Delay(last_time + FPS - now_time);	/* delay some time */

      flipscreen();
    }

  return done;
}

int le_init()
{
  int i;
  level_subsets = dsubdirs("/levels", "info");

  active_tm = TM_IA;
  
  le_show_grid = true;

  /*  level_changed = NO;*/
  fire = DOWN;
  done = 0;
  le_frame = 0;	/* support for frames in some tiles, like waves and bad guys */
  le_level_changed = false;
  le_current_level = NULL;

  le_current_tile = 0;
  le_mouse_pressed[LEFT] = false;
  le_mouse_pressed[RIGHT] = false;

  le_selection = new Surface(datadir + "/images/leveleditor/select.png", USE_ALPHA);

  select_tilegroup_menu_effect.init(false);

  /* Load buttons */
  le_save_level_bt = new Button("/images/icons/save.png","Save level", SDLK_F6,screen->w-64,32);
  le_exit_bt = new Button("/images/icons/exit.png","Exit", SDLK_F6,screen->w-32,32);
  le_next_level_bt = new Button("/images/icons/up.png","Next level", SDLK_PAGEUP,screen->w-64,0);
  le_previous_level_bt = new Button("/images/icons/down.png","Previous level",SDLK_PAGEDOWN,screen->w-32,0);
  le_rubber_bt = new Button("/images/icons/rubber.png","Rubber",SDLK_DELETE,screen->w-32,48);
  le_select_mode_one_bt = new Button ("/images/icons/select-mode1.png","Select single tile",SDLK_F3,screen->w-64,48);
  le_select_mode_two_bt = new Button("/images/icons/select-mode2.png","Select multiple tiles",SDLK_F3,screen->w-64,64);
  le_test_level_bt = new Button("/images/icons/test-level.png","Test level",SDLK_F4,screen->w-64,screen->h - 64);
  le_settings_bt = new Button("/images/icons/settings.png","Level settings",SDLK_F5,screen->w-32,screen->h - 64);
  le_move_left_bt = new Button("/images/icons/left.png","Move left",SDLK_LEFT,0,0);
  le_move_right_bt = new Button("/images/icons/right.png","Move right",SDLK_RIGHT,screen->w-80,0);
  le_tilegroup_bt = new Button("/images/icons/tilegroup.png","Select Tilegroup", SDLK_F7,screen->w-64,80);
  
  le_tilemap_panel = new ButtonPanel(screen->w-64,screen->h-32,32,32);
  le_tilemap_panel->set_button_size(32,10);
  le_tilemap_panel->additem(new Button("/images/icons/bkgrd.png","Background",SDLK_F4,0,0),TM_BG);
  le_tilemap_panel->additem(new Button("/images/icons/intact.png","Interactive",SDLK_F4,0,0),TM_IA);
  le_tilemap_panel->additem(new Button("/images/icons/frgrd.png","Foreground",SDLK_F4,0,0),TM_FG); 
  
  leveleditor_menu = new Menu();
  subset_load_menu = new Menu();
  subset_new_menu  = new Menu();
  subset_settings_menu = new Menu();
  level_settings_menu  = new Menu();
  select_tilegroup_menu  = new Menu();

  leveleditor_menu->additem(MN_LABEL,"Level Editor Menu",0,0);
  leveleditor_menu->additem(MN_HL,"",0,0);
  leveleditor_menu->additem(MN_ACTION,"Return To Level Editor",0,0,MNID_RETURNLEVELEDITOR);
  leveleditor_menu->additem(MN_DEACTIVE,"Level Subset Settings",0,subset_settings_menu,MNID_SUBSETSETTINGS);
  leveleditor_menu->additem(MN_GOTO,"Load Level Subset",0,subset_load_menu);
  leveleditor_menu->additem(MN_GOTO,"New Level Subset",0,subset_new_menu);
  leveleditor_menu->additem(MN_HL,"",0,0);
  leveleditor_menu->additem(MN_ACTION,"Quit Level Editor",0,0,MNID_QUITLEVELEDITOR);

  Menu::set_current(leveleditor_menu);
  
  subset_load_menu->additem(MN_LABEL, "Load Level Subset", 0, 0);
  subset_load_menu->additem(MN_HL, "", 0, 0);

  for(i = 0; i < level_subsets.num_items; ++i)
    {
      subset_load_menu->additem(MN_ACTION,level_subsets.item[i],0,0);
    }
  subset_load_menu->additem(MN_HL,"",0,0);
  subset_load_menu->additem(MN_BACK,"Back",0,0);

  subset_new_menu->additem(MN_LABEL,"New Level Subset",0,0);
  subset_new_menu->additem(MN_HL,"",0,0);
  subset_new_menu->additem(MN_TEXTFIELD,"Enter Name",0,0);
  subset_new_menu->additem(MN_ACTION,"Create",0,0);
  subset_new_menu->additem(MN_HL,"",0,0);
  subset_new_menu->additem(MN_BACK,"Back",0,0);

  subset_settings_menu->additem(MN_LABEL,"Level Subset Settings",0,0);
  subset_settings_menu->additem(MN_HL,"",0,0);
  subset_settings_menu->additem(MN_TEXTFIELD,"Title",0,0);
  subset_settings_menu->additem(MN_TEXTFIELD,"Description",0,0);
  subset_settings_menu->additem(MN_HL,"",0,0);
  subset_settings_menu->additem(MN_ACTION,"Save Changes",0,0);
  subset_settings_menu->additem(MN_HL,"",0,0);
  subset_settings_menu->additem(MN_BACK,"Back",0,0);

  level_settings_menu->arrange_left = true;
  level_settings_menu->additem(MN_LABEL,"Level Settings",0,0);
  level_settings_menu->additem(MN_HL,"",0,0);
  level_settings_menu->additem(MN_TEXTFIELD,"Name    ",0,0);
  level_settings_menu->additem(MN_TEXTFIELD,"Author  ",0,0);
  level_settings_menu->additem(MN_STRINGSELECT,"Theme   ",0,0);
  level_settings_menu->additem(MN_STRINGSELECT,"Song    ",0,0);
  level_settings_menu->additem(MN_STRINGSELECT,"Bg-Image",0,0);
  level_settings_menu->additem(MN_NUMFIELD,"Length ",0,0);
  level_settings_menu->additem(MN_NUMFIELD,"Time   ",0,0);
  level_settings_menu->additem(MN_NUMFIELD,"Gravity",0,0);
  level_settings_menu->additem(MN_NUMFIELD,"Top Red    ",0,0);
  level_settings_menu->additem(MN_NUMFIELD,"Top Green  ",0,0);
  level_settings_menu->additem(MN_NUMFIELD,"Top Blue   ",0,0);
  level_settings_menu->additem(MN_NUMFIELD,"Bottom Red ",0,0);
  level_settings_menu->additem(MN_NUMFIELD,"Bottom Green",0,0);
  level_settings_menu->additem(MN_NUMFIELD,"Bottom Blue",0,0);
  level_settings_menu->additem(MN_HL,"",0,0);
  level_settings_menu->additem(MN_ACTION,"Apply Changes",0,0,MNID_APPLY);

  select_tilegroup_menu->arrange_left = true;
  select_tilegroup_menu->additem(MN_LABEL,"Select Tilegroup",0,0);
  select_tilegroup_menu->additem(MN_HL,"",0,0);
  std::vector<TileGroup>* tilegroups = TileManager::tilegroups();
  for(std::vector<TileGroup>::iterator it = tilegroups->begin(); it != tilegroups->end(); ++it )
    {

      select_tilegroup_menu->additem(MN_ACTION,const_cast<char*>((*it).name.c_str()),0,0);
      tilegroups_map[(*it).name] = new ButtonPanel(screen->w - 64,96, 64, 318);
      i = 0;
      for(std::vector<int>::iterator sit = (*it).tiles.begin(); sit != (*it).tiles.end(); ++sit, ++i)
        tilegroups_map[(*it).name]->additem(new Button(const_cast<char*>(("images/tilesets/" + TileManager::instance()->get(*sit)->filenames[0]).c_str()), const_cast<char*>((*it).name.c_str()),(SDLKey)(i+'a'),0,0,32,32),(*sit));
    }
  select_tilegroup_menu->additem(MN_HL,"",0,0);

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  return 0;
}

void update_level_settings_menu()
{
  char str[80];
  int i;

  level_settings_menu->item[2].change_input(le_current_level->name.c_str());
  level_settings_menu->item[3].change_input(le_current_level->author.c_str());
  sprintf(str,"%d",le_current_level->width);

  string_list_copy(level_settings_menu->item[4].list, dsubdirs("images/themes", "solid0.png"));
  string_list_copy(level_settings_menu->item[5].list, dfiles("music/",NULL, "-fast"));
  string_list_copy(level_settings_menu->item[6].list, dfiles("images/background",NULL, NULL));
  string_list_add_item(level_settings_menu->item[6].list,"");
  if((i = string_list_find(level_settings_menu->item[4].list,le_current_level->theme.c_str())) != -1)
    level_settings_menu->item[3].list->active_item = i;
  if((i = string_list_find(level_settings_menu->item[5].list,le_current_level->song_title.c_str())) != -1)
    level_settings_menu->item[4].list->active_item = i;
  if((i = string_list_find(level_settings_menu->item[6].list,le_current_level->bkgd_image.c_str())) != -1)
    level_settings_menu->item[5].list->active_item = i;

  level_settings_menu->item[7].change_input(str);
  sprintf(str,"%d",le_current_level->time_left);
  level_settings_menu->item[8].change_input(str);
  sprintf(str,"%2.0f",le_current_level->gravity);
  level_settings_menu->item[9].change_input(str);
  sprintf(str,"%d",le_current_level->bkgd_top.red);
  level_settings_menu->item[10].change_input(str);
  sprintf(str,"%d",le_current_level->bkgd_top.green);
  level_settings_menu->item[11].change_input(str);
  sprintf(str,"%d",le_current_level->bkgd_top.blue);
  level_settings_menu->item[12].change_input(str);
  sprintf(str,"%d",le_current_level->bkgd_bottom.red);
  level_settings_menu->item[13].change_input(str);
  sprintf(str,"%d",le_current_level->bkgd_bottom.green);
  level_settings_menu->item[14].change_input(str);
  sprintf(str,"%d",le_current_level->bkgd_bottom.blue);
  level_settings_menu->item[15].change_input(str);
}

void update_subset_settings_menu()
{
  subset_settings_menu->item[2].change_input(le_level_subset.title.c_str());
  subset_settings_menu->item[3].change_input(le_level_subset.description.c_str());
}

void apply_level_settings_menu()
{
  int i;
  i = false;

  le_current_level->name = level_settings_menu->item[2].input;
  le_current_level->author = level_settings_menu->item[3].input;

  if(le_current_level->bkgd_image.compare(string_list_active(level_settings_menu->item[6].list)) != 0)
    {
      le_current_level->bkgd_image = string_list_active(level_settings_menu->item[6].list);
      i = true;
    }

  if(le_current_level->theme.compare(string_list_active(level_settings_menu->item[4].list)) != 0)
    {
      le_current_level->theme = string_list_active(level_settings_menu->item[4].list);
      i = true;
    }

  if(i)
    {
      le_current_level->free_gfx();
      le_current_level->load_gfx();
    }

  le_current_level->song_title = string_list_active(level_settings_menu->item[5].list);

  le_current_level->change_size(atoi(level_settings_menu->item[7].input));
  le_current_level->time_left = atoi(level_settings_menu->item[8].input);
  le_current_level->gravity = atof(level_settings_menu->item[9].input);
  le_current_level->bkgd_top.red = atoi(level_settings_menu->item[10].input);
  le_current_level->bkgd_top.green = atoi(level_settings_menu->item[11].input);
  le_current_level->bkgd_top.blue = atoi(level_settings_menu->item[12].input);
  le_current_level->bkgd_bottom.red = atoi(level_settings_menu->item[13].input);
  le_current_level->bkgd_bottom.green = atoi(level_settings_menu->item[14].input);
  le_current_level->bkgd_bottom.blue = atoi(level_settings_menu->item[15].input);
}

void save_subset_settings_menu()
{
  le_level_subset.title = subset_settings_menu->item[2].input;
  le_level_subset.description = subset_settings_menu->item[3].input;
  le_level_subset.save();
}

void le_goto_level(int levelnb)
{
  le_world.arrays_free();

  le_current_level->cleanup();
  if(le_current_level->load(le_level_subset.name.c_str(), levelnb) != 0)
    {
      le_current_level->load(le_level_subset.name.c_str(), le_level);
    }
  else
    {
      le_level = levelnb;
    }

  le_set_defaults();

  le_current_level->free_gfx();
  le_current_level->load_gfx();

  le_world.activate_bad_guys();
}

void le_quit(void)
{
  /*if(level_changed == true)
    if(askforsaving() == CANCEL)
      return;*/ //FIXME

  SDL_EnableKeyRepeat(0, 0);    // disables key repeating

  delete le_selection;
  delete leveleditor_menu;
  delete subset_load_menu;
  delete subset_new_menu;
  delete subset_settings_menu;
  delete level_settings_menu;
  delete select_tilegroup_menu;
  delete le_save_level_bt;
  delete le_exit_bt;
  delete le_test_level_bt;
  delete le_next_level_bt;
  delete le_previous_level_bt;
  delete le_move_right_bt;
  delete le_move_left_bt;
  delete le_rubber_bt;
  delete le_select_mode_one_bt;
  delete le_select_mode_two_bt;
  delete le_settings_bt;
  delete le_tilegroup_bt;
  delete le_tilemap_panel;

  if(le_current_level != NULL)
    {
      le_current_level->free_gfx();
      le_current_level->cleanup();
      le_world.arrays_free();
    }
}

void le_drawinterface()
{
  int x,y;
  char str[80];

  if(le_current_level != NULL)
    {
      /* draw a grid (if selected) */
      if(le_show_grid)
        {
          for(x = 0; x < 19; x++)
            fillrect(x*32 - ((int)pos_x % 32), 0, 1, screen->h, 225, 225, 225,255);
          for(y = 0; y < 15; y++)
            fillrect(0, y*32, screen->w - 32, 1, 225, 225, 225,255);
        }
    }

  if(le_selection_mode == CURSOR)
    le_selection->draw( cursor_x - pos_x, cursor_y);
  else if(le_selection_mode == SQUARE)
    {
      int w, h;
      le_highlight_selection();
      /* draw current selection */
      w = selection.x2 - selection.x1;
      h = selection.y2 - selection.y1;
      fillrect(selection.x1 - pos_x, selection.y1, w, SELECT_W, SELECT_CLR);
      fillrect(selection.x1 - pos_x + w, selection.y1, SELECT_W, h, SELECT_CLR);
      fillrect(selection.x1 - pos_x, selection.y1 + h, w, SELECT_W, SELECT_CLR);
      fillrect(selection.x1 - pos_x, selection.y1, SELECT_W, h, SELECT_CLR);
    }


  /* draw button bar */
  fillrect(screen->w - 64, 0, 64, screen->h, 50, 50, 50,255);
  Tile::draw(19 * 32, 14 * 32, le_current_tile);
  
  	if(TileManager::instance()->get(le_current_tile)->editor_images.size() > 0)
	TileManager::instance()->get(le_current_tile)->editor_images[0]->draw( 19 * 32, 14 * 32);

  if(le_current_level != NULL)
    {
      le_save_level_bt->draw();
      le_exit_bt->draw();
      le_test_level_bt->draw();
      le_next_level_bt->draw();
      le_previous_level_bt->draw();
      le_rubber_bt->draw();
      le_select_mode_one_bt->draw();
      le_select_mode_two_bt->draw();
      le_settings_bt->draw();
      le_move_right_bt->draw();
      le_move_left_bt->draw();
      le_tilegroup_bt->draw();
      if(!cur_tilegroup.empty())
      tilegroups_map[cur_tilegroup]->draw();
      le_tilemap_panel->draw();

      sprintf(str, "%d/%d", le_level,le_level_subset.levels);
      white_text->drawf(str, -10, 16, A_RIGHT, A_TOP, 0);

      white_small_text->draw("F1 for Help", 10, 430, 1);
    }
  else
    {
      if(!Menu::current())
        white_small_text->draw("No Level Subset loaded - Press ESC and choose one in the menu", 10, 430, 1);
      else
        white_small_text->draw("No Level Subset loaded", 10, 430, 1);
    }

}

void le_drawlevel()
{
  unsigned int y,x,i,s;
  Uint8 a;

  /* Draw the real background */
  if(le_current_level->bkgd_image[0] != '\0')
    {
      s = pos_x / 30;
      le_current_level->img_bkgd->draw_part(s,0,0,0,
                                            le_current_level->img_bkgd->w - s - 32, le_current_level->img_bkgd->h);
      le_current_level->img_bkgd->draw_part(0,0,screen->w - s - 32 ,0,s,
                                            le_current_level->img_bkgd->h);
    }
  else
    {
          drawgradient(le_current_level->bkgd_top, le_current_level->bkgd_bottom);
    }

  /*       clearscreen(current_level.bkgd_red, current_level.bkgd_green, current_level.bkgd_blue); */

  for (y = 0; y < 15; ++y)
    for (x = 0; x < 20; ++x)
      {
      
	if(active_tm == TM_BG)
	a = 255;
	else
	a = 128;
      
	Tile::draw(32*x - fmodf(pos_x, 32), y * 32, le_current_level->bg_tiles[y][x + (int)(pos_x / 32)],a);
	
	if(active_tm == TM_IA)
	a = 255;
	else
	a = 128;
	
        Tile::draw(32*x - fmodf(pos_x, 32), y * 32, le_current_level->ia_tiles[y][x + (int)(pos_x / 32)],a);

	if(active_tm == TM_FG)
	a = 255;
	else
	a = 128;
	
	Tile::draw(32*x - fmodf(pos_x, 32), y * 32, le_current_level->fg_tiles[y][x + (int)(pos_x / 32)],a);
	
        /* draw whats inside stuff when cursor is selecting those */
        /* (draw them all the time - is this the right behaviour?) */
	if(TileManager::instance()->get(le_current_level->ia_tiles[y][x + (int)(pos_x / 32)])->editor_images.size() > 0)
	TileManager::instance()->get(le_current_level->ia_tiles[y][x + (int)(pos_x / 32)])->editor_images[0]->draw( x * 32 - ((int)pos_x % 32), y*32);

      }

  /* Draw the Bad guys: */
  for (i = 0; i < le_world.bad_guys.size(); ++i)
    {
      /* to support frames: img_bsod_left[(frame / 5) % 4] */
      
      scroll_x = pos_x;
      le_world.bad_guys[i].draw();
    }


  /* Draw the player: */
  /* for now, the position is fixed at (100, 240) */
  largetux.walk_right->draw( 100 - pos_x, 240);
}

void le_checkevents()
{
  SDLKey key;
  SDLMod keymod;
  Button* pbutton;
  int x,y;

  keymod = SDL_GetModState();

  while(SDL_PollEvent(&event))
    {
      if (Menu::current())
        {
          Menu::current()->event(event);
        }
      else
        {
          mouse_cursor->set_state(MC_NORMAL);

          /* testing SDL_KEYDOWN, SDL_KEYUP and SDL_QUIT events*/
          if(event.type == SDL_KEYDOWN 
             || ((event.type == SDL_MOUSEBUTTONDOWN || SDL_MOUSEMOTION)
                 && (event.motion.x > 0 
                     && event.motion.x < screen->w - 64 &&
                     event.motion.y > 0 && event.motion.y < screen->h)))
            {
              switch(event.type)
                {
                case SDL_KEYDOWN:	// key pressed
                  key = event.key.keysym.sym;
                  switch(key)
                    {
                    case SDLK_ESCAPE:
                      Menu::set_current(leveleditor_menu);
                    case SDLK_LEFT:
                      if(fire == DOWN)
                        cursor_x -= KEY_CURSOR_SPEED;
                      else
                        cursor_x -= KEY_CURSOR_FASTSPEED;

                      if(cursor_x < pos_x + MOUSE_LEFT_MARGIN)
                        pos_x = cursor_x - MOUSE_LEFT_MARGIN;

                      break;
                    case SDLK_RIGHT:
                      if(fire == DOWN)
                        cursor_x += KEY_CURSOR_SPEED;
                      else
                        cursor_x += KEY_CURSOR_FASTSPEED;

                      if(cursor_x > pos_x + MOUSE_RIGHT_MARGIN-32)
                        pos_x = cursor_x - MOUSE_RIGHT_MARGIN+32;

                      break;
                    case SDLK_UP:
                      if(fire == DOWN)
                        cursor_y -= KEY_CURSOR_SPEED;
                      else
                        cursor_y -= KEY_CURSOR_FASTSPEED;

                      if(cursor_y < 0)
                        cursor_y = 0;
                      break;
                    case SDLK_DOWN:
                      if(fire == DOWN)
                        cursor_y += KEY_CURSOR_SPEED;
                      else
                        cursor_y += KEY_CURSOR_FASTSPEED;

                      if(cursor_y > screen->h-32)
                        cursor_y = screen->h-32;
                      break;
                    case SDLK_LCTRL:
                      fire =UP;
                      break;
                    case SDLK_F1:
                      le_showhelp();
                      break;
                    case SDLK_HOME:
                      cursor_x = 0;
                      pos_x = cursor_x;
                      break;
                    case SDLK_END:
                      cursor_x = (le_current_level->width * 32) - 32;
                      pos_x = cursor_x;
                      break;
                    case SDLK_F9:
                      le_show_grid = !le_show_grid;
                      break;
                    default:
                      break;
                    }
                  break;
                case SDL_KEYUP:	/* key released */
                  switch(event.key.keysym.sym)
                    {
                    case SDLK_LCTRL:
                      fire = DOWN;
                      break;
                    default:
                      break;
                    }
                  break;
                case SDL_MOUSEBUTTONDOWN:
                  if(event.button.button == SDL_BUTTON_LEFT)
                    {
                      le_mouse_pressed[LEFT] = true;

                      selection.x1 = event.motion.x + pos_x;
                      selection.y1 = event.motion.y;
                      selection.x2 = event.motion.x + pos_x;
                      selection.y2 = event.motion.y;
                    }
                  else if(event.button.button == SDL_BUTTON_RIGHT)
                    {
                      le_mouse_pressed[RIGHT] = true;
                    }
                  break;
                case SDL_MOUSEBUTTONUP:
                  if(event.button.button == SDL_BUTTON_LEFT)
                    le_mouse_pressed[LEFT] = false;
                  else if(event.button.button == SDL_BUTTON_RIGHT)
                    le_mouse_pressed[RIGHT] = false;
                  break;
                case SDL_MOUSEMOTION:

                  if(!Menu::current())
                    {
                      x = event.motion.x;
                      y = event.motion.y;

                      cursor_x = ((int)(pos_x + x) / 32) * 32;
                      cursor_y = ((int) y / 32) * 32;

                      if(le_mouse_pressed[LEFT])
                        {
                          selection.x2 = x + pos_x;
                          selection.y2 = y;
                        }

                      if(le_mouse_pressed[RIGHT])
                        {
                          pos_x += -1 * event.motion.xrel;
                        }
                    }
                  break;
                case SDL_QUIT:	// window closed
                  done = 1;
                  break;
                default:
                  break;
                }
            }
        }

      if(le_current_level != NULL)
        {
          if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP || ((event.type == SDL_MOUSEBUTTONDOWN || SDL_MOUSEMOTION) && (event.motion.x > screen->w-64 && event.motion.x < screen->w &&
                                                                                                                                 event.motion.y > 0 && event.motion.y < screen->h)))
            {
              le_mouse_pressed[LEFT] = false;
              le_mouse_pressed[RIGHT] = false;

              if(!Menu::current())
                {
                  /* Check for button events */
                  le_test_level_bt->event(event);
                  if(le_test_level_bt->get_state() == BUTTON_CLICKED)
                    le_testlevel();
                  le_save_level_bt->event(event);
                  if(le_save_level_bt->get_state() == BUTTON_CLICKED)
                    le_current_level->save(le_level_subset.name.c_str(),le_level);
                  le_exit_bt->event(event);
                  if(le_exit_bt->get_state() == BUTTON_CLICKED)
                    {
                      Menu::set_current(leveleditor_menu);
                    }
                  le_next_level_bt->event(event);
                  if(le_next_level_bt->get_state() == BUTTON_CLICKED)
                    {
                      if(le_level < le_level_subset.levels)
                        {
                          le_goto_level(++le_level);
                        }
                      else
                        {
                          Level new_lev;
                          char str[1024];
                          int d = 0;
                          sprintf(str,"Level %d doesn't exist.",le_level+1);
                          white_text->drawf(str,0,-18,A_HMIDDLE,A_VMIDDLE,2);
                          white_text->drawf("Do you want to create it?",0,0,A_HMIDDLE,A_VMIDDLE,2);
                          red_text->drawf("(Y)es/(N)o",0,20,A_HMIDDLE,A_VMIDDLE,2);
                          flipscreen();
                          while(d == 0)
                            {
                              while(SDL_PollEvent(&event))
                                switch(event.type)
                                  {
                                  case SDL_KEYDOWN:		// key pressed
                                    switch(event.key.keysym.sym)
                                      {
                                      case SDLK_y:
                                        new_lev.init_defaults();
                                        new_lev.save(le_level_subset.name.c_str(),++le_level);
                                        le_level_subset.levels = le_level;
                                        le_goto_level(le_level);
                                        d = 1;
                                        break;
                                      case SDLK_n:
                                        d = 1;
                                        break;
                                      default:
                                        break;
                                      }
                                    break;
                                  default:
                                    break;
                                  }
                              SDL_Delay(50);
                            }
                        }
                    }
                  le_previous_level_bt->event(event);
                  if(le_previous_level_bt->get_state() == BUTTON_CLICKED)
                    {
                      if(le_level > 1)
                        le_goto_level(--le_level);
                    }
                  le_rubber_bt->event(event);
                  if(le_rubber_bt->get_state() == BUTTON_CLICKED)
                    le_current_tile = 0;
                  le_select_mode_one_bt->event(event);
                  if(le_select_mode_one_bt->get_state() == BUTTON_CLICKED)
                    le_selection_mode = CURSOR;
                  le_select_mode_two_bt->event(event);
                  if(le_select_mode_two_bt->get_state() == BUTTON_CLICKED)
                    le_selection_mode = SQUARE;

                  le_tilegroup_bt->event(event);
                  if(le_tilegroup_bt->get_state() == BUTTON_CLICKED)
                    {
                      Menu::set_current(select_tilegroup_menu);
                      select_tilegroup_menu_effect.start(200);
                      select_tilegroup_menu->set_pos(screen->w - 64,100,-0.5,0.5);
                    }

                  le_settings_bt->event(event);
                  if(le_settings_bt->get_state() == BUTTON_CLICKED)
                    {
                      update_level_settings_menu();
                      Menu::set_current(level_settings_menu);
                    }
		  if(!cur_tilegroup.empty())
                    if((pbutton = tilegroups_map[cur_tilegroup]->event(event)) != NULL)
                      {
                        if(pbutton->get_state() == BUTTON_CLICKED)
                          {
                            le_current_tile = pbutton->get_tag();
                          }
                      }
		  if((pbutton = le_tilemap_panel->event(event)) != NULL)
                    {
                      if(pbutton->get_state() == BUTTON_CLICKED)
                        {
                          active_tm = static_cast<TileMapType>(pbutton->get_tag());
                        }
                    }
                }
              else
                {
                  le_settings_bt->event(event);
                  if(le_settings_bt->get_state() == BUTTON_CLICKED)
                    {
                      Menu::set_current(0);
                    }
                  le_tilegroup_bt->event(event);
                  if(le_tilegroup_bt->get_state() == BUTTON_CLICKED)
                    {
                      Menu::set_current(0);
                    }
                }
            }
          
          if(!Menu::current())
            {
              le_move_left_bt->event(event);
              le_move_right_bt->event(event);

              if(le_mouse_pressed[LEFT])
                {
                  le_change(cursor_x, cursor_y, active_tm, le_current_tile);
                }
            }
        }
    }
  if(!Menu::current())
    {
      if(le_move_left_bt->get_state() == BUTTON_PRESSED)
        {
          pos_x -= 192;
        }
      else if(le_move_left_bt->get_state() == BUTTON_HOVER)
        {
          pos_x -= 64;
        }

      if(le_move_right_bt->get_state() == BUTTON_PRESSED)
        {
          pos_x += 192;
        }
      else if(le_move_right_bt->get_state() == BUTTON_HOVER)
        {
          pos_x += 64;
        }
    }

}

void le_highlight_selection()
{
  int x1, x2, y1, y2;

  if(selection.x1 < selection.x2)
    {
      x1 = selection.x1;
      x2 = selection.x2;
    }
  else
    {
      x1 = selection.x2;
      x2 = selection.x1;
    }
  if(selection.y1 < selection.y2)
    {
      y1 = selection.y1;
      y2 = selection.y2;
    }
  else
    {
      y1 = selection.y2;
      y2 = selection.y1;
    }

  x1 /= 32;
  x2 /= 32;
  y1 /= 32;
  y2 /= 32;

  fillrect(x1*32-pos_x, y1*32,32* (x2 - x1 + 1),32 * (y2 - y1 + 1),173,234,177,103);
}

void le_change(float x, float y, int tm, unsigned int c)
{
  if(le_current_level != NULL)
    {
      int xx,yy;
      int x1, x2, y1, y2;
      unsigned int i;

      /*  level_changed = true; */

      switch(le_selection_mode)
        {
        case CURSOR:
          le_current_level->change(x,y,tm,c);

          yy = ((int)y / 32);
          xx = ((int)x / 32);

          /* if there is a bad guy over there, remove it */
          for(i = 0; i < le_world.bad_guys.size(); ++i)
            if(xx == le_world.bad_guys[i].base.x/32 && yy == le_world.bad_guys[i].base.y/32)
              le_world.bad_guys.erase(le_world.bad_guys.begin() + i);

          if(c == '0')  /* if it's a bad guy */
            le_world.add_bad_guy(xx*32, yy*32, BAD_SNOWBALL);
          else if(c == '1')
            le_world.add_bad_guy(xx*32, yy*32, BAD_MRICEBLOCK);
          else if(c == '2')
            le_world.add_bad_guy(xx*32, yy*32, BAD_JUMPY);

          break;
        case SQUARE:
          if(selection.x1 < selection.x2)
            {
              x1 = selection.x1;
              x2 = selection.x2;
            }
          else
            {
              x1 = selection.x2;
              x2 = selection.x1;
            }
          if(selection.y1 < selection.y2)
            {
              y1 = selection.y1;
              y2 = selection.y2;
            }
          else
            {
              y1 = selection.y2;
              y2 = selection.y1;
            }

          x1 /= 32;
          x2 /= 32;
          y1 /= 32;
          y2 /= 32;

          /* if there is a bad guy over there, remove it */
          for(std::vector<BadGuy>::iterator i = le_world.bad_guys.begin(); 
              i != le_world.bad_guys.end(); /* will be at end of loop */) {
            if(i->base.x/32 >= x1 && i->base.x/32 <= x2
               && i->base.y/32 >= y1 && i->base.y/32 <= y2) {
              i = le_world.bad_guys.erase(i);
              continue;
            } else {
              ++i;
            }
          }

          for(xx = x1; xx <= x2; xx++)
            for(yy = y1; yy <= y2; yy++)
              {
                le_current_level->change(xx*32, yy*32, tm, c);

                if(c == '0')  // if it's a bad guy
                  le_world.add_bad_guy(xx*32, yy*32, BAD_SNOWBALL);
                else if(c == '1')
                  le_world.add_bad_guy(xx*32, yy*32, BAD_MRICEBLOCK);
                else if(c == '2')
                  le_world.add_bad_guy(xx*32, yy*32, BAD_JUMPY);
              }
          break;
        default:
          break;
        }
    }
}

void le_testlevel()
{
  le_current_level->save("test", le_level);
  
  GameSession session("test",le_level, ST_GL_TEST);
  session.run();

  music_manager->halt_music();

  Menu::set_current(leveleditor_menu);
  le_world.arrays_free();
  le_current_level->load_gfx();
  le_world.activate_bad_guys();
}

void le_showhelp()
{
  SDL_Event event;
  unsigned int i, done_;
  char *text[] = {
                   "  - This is SuperTux's built-in level editor -",
                   "It has been designed to be light and easy to use from the start.",
                   "",
                   "When you first load the level editor you are given a menu where you",
                   "can load level subsets, create a new level subset, edit the current",
                   "subset's settings, or simply quit the editor. You can access this menu",
                   "from the level editor at any time by pressing the escape key.",
                   "",
                   "To your right is your button bar. The center of this contains many",
                   "tiles you can use to make your level. To select a tile, click on it",
                   "with your left mouse button; your selection will be shown in the",
                   "bottom right corner of the button box. Click anywhere on your level",
                   "with the left mouse button to place that tile down. If you right click",
                   "a tile in the button bar, you can find out what its keyboard shortcut",
                   "is. The three buttons FGD, BGD and EMY let you pick from foreground,",
                   "background, and enemy tiles. The eraser lets you remove tiles.",
                   "The left and right arrow keys scroll back and forth through your level.",
                   "The button with the wrench and screwdriver, lets you change the",
                   "settings of your level, including how long it is or what music it will",
                   "play. When you are ready to give your level a test, click on the little",
                   "running Tux. If you like the changes you have made to your level,",
                   "press the red save key to keep them.",
                   "To change which level in your subset you are editing, press the white",
                   "up and down arrow keys at the top of the button box.",
                   "",
                   "Have fun making levels! If you make some good ones, send them to us on",
                   "the SuperTux mailing list!",
                   "- SuperTux team"
                 };


  blue_text->drawf("- Help -", 0, 30, A_HMIDDLE, A_TOP, 2);

  for(i = 0; i < sizeof(text)/sizeof(char *); i++)
    white_small_text->draw(text[i], 5, 80+(i*white_small_text->h), 1);

  gold_text->drawf("Press Any Key to Continue", 0, 440, A_HMIDDLE, A_TOP, 1);

  flipscreen();

  done_ = 0;

  while(done_ == 0)
    {
      done_ = wait_for_event(event);
      SDL_Delay(50);
    }
}
