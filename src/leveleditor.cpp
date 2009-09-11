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
#ifndef NOSOUND
#include "music_manager.h"
#endif

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
/* crucial ones (main loop) */
int le_init();
void le_quit();
int le_load_level_subset(char *filename);
void le_drawlevel();
void le_drawinterface();
void le_checkevents();
void le_change(float x, float y, int tm, unsigned int c);
void le_testlevel();
void le_showhelp();
void le_set_defaults(void);
void le_activate_bad_guys(void);
void le_goto_level(int levelnb);
void le_highlight_selection();

void apply_level_settings_menu();
void update_subset_settings_menu();
void save_subset_settings_menu();

struct TileOrObject
{
  TileOrObject() : tile(0), obj(NULL) { is_tile = true; };

  void Tile(unsigned int set_to) { tile = set_to; is_tile = true; }
  void Object(GameObject* pobj) { obj = pobj; is_tile = false; }
  //Returns true for a tile
  bool IsTile() { return is_tile; };
  //Returns true for a GameObject
  bool IsObject() { return !is_tile; };


  void Init() { tile = 0; obj = NULL; is_tile = true; };

  bool is_tile; //true for tile (false for object)
  unsigned int tile;
  GameObject* obj;
};

/* leveleditor internals */
static string_list_type level_subsets;
static bool le_level_changed;  /* if changes, ask for saving, when quiting*/
static bool show_minimap;
static bool show_selections;
static bool le_help_shown;
static int pos_x, cursor_x, cursor_y, fire;
static int le_level;
static World* le_world;
static LevelSubset* le_level_subset;
static int le_show_grid;
static int le_frame;
static Surface* le_selection;
static int done;
static TileOrObject le_current;
static bool le_mouse_pressed[2];
static bool le_mouse_clicked[2];
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
static Button* le_objects_bt;
static Button* le_object_select_bt;
static Button* le_object_properties_bt;
static ButtonPanel* le_tilemap_panel;
static Menu* leveleditor_menu;
static Menu* subset_load_menu;
static Menu* subset_new_menu;
static Menu* subset_settings_menu;
static Menu* level_settings_menu;
static Menu* select_tilegroup_menu;
static Menu* select_objects_menu;
static Timer select_tilegroup_menu_effect;
static Timer select_objects_menu_effect;
static Timer display_level_info;
typedef std::map<std::string, ButtonPanel*> ButtonPanelMap;
static ButtonPanelMap tilegroups_map;
static ButtonPanelMap objects_map;
static std::string cur_tilegroup;
static std::string cur_objects;
static MouseCursor* mouse_select_object;
static GameObject* selected_game_object;

static square selection;
static int le_selection_mode;
static SDL_Event event;
TileMapType active_tm;

int leveleditor(char* filename)
{
  int last_time, now_time, i;

  le_level = 1;

  if(le_init() != 0)
    return 1;

  /* Clear screen: */

  clearscreen(0, 0, 0);
  updatescreen();

#ifndef NOSOUND
  music_manager->halt_music();
#endif

  while (SDL_PollEvent(&event))
  {}

  if(filename != NULL)
    if(le_load_level_subset(filename))
      return 1;

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
                                       66,-0.5,0.5);
      }
      else
        select_tilegroup_menu->set_pos(screen->w - 64,66,-0.5,0.5);
    }
    else if(Menu::current() == select_objects_menu)
    {
      if(select_objects_menu_effect.check())
      {
        select_objects_menu->set_pos(screen->w - 64 + select_objects_menu_effect.get_left(),82,-0.5,0.5);
      }
      else
        select_objects_menu->set_pos(screen->w - 64,82,-0.5,0.5);
    }

    if(le_world != NULL)
    {
      /* making events results to be in order */
      if(pos_x < 0)
        pos_x = 0;
      if(pos_x > (le_world->get_level()->width * 32 + 2*32) - screen->w)
        pos_x = (le_world->get_level()->width * 32 +2*32) - screen->w;

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
          if(le_world != NULL)
            Menu::set_current(0);
          else
            Menu::set_current(leveleditor_menu);
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
        case MNID_APPLY:
          apply_level_settings_menu();
          Menu::set_current(NULL);
          break;

        default:
          break;
        }
      }
      else if(menu == select_tilegroup_menu)
      {
        int it = -1;
        switch (it = select_tilegroup_menu->check())
        {
        default:
          if(it >= 0)
          {
            cur_tilegroup = select_tilegroup_menu->get_item_by_id(it).text;
            Menu::set_current(0);
            cur_objects = "";

          }
          break;
        }
      }
      else if(menu == select_objects_menu)
      {
        int it = -1;
        switch (it = select_objects_menu->check())
        {
        default:
          if(it >= 0)
          {
            cur_objects = select_objects_menu->get_item_by_id(it).text;
            cur_tilegroup = "";

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
          if(i >= 1)
          {
            if(le_load_level_subset(level_subsets.item[i-1]))
              return 1;
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
          case MNID_CREATESUBSET:
            LevelSubset::create(subset_new_menu->get_item_by_id(MNID_SUBSETNAME).input);
            le_level_subset->load(subset_new_menu->get_item_by_id(MNID_SUBSETNAME).input);
            leveleditor_menu->get_item_by_id(MNID_SUBSETSETTINGS).kind = MN_GOTO;
            le_goto_level(1);
            subset_new_menu->get_item_by_id(MNID_SUBSETNAME).change_input("");

            Menu::set_current(subset_settings_menu);
            break;
          }
        }
      }
      else if(menu == subset_settings_menu)
      {
        if(le_level_subset->title.compare(subset_settings_menu->get_item_by_id(MNID_SUBSETTITLE).input) == 0 && le_level_subset->description.compare(subset_settings_menu->get_item_by_id(MNID_SUBSETDESCRIPTION).input) == 0  )
          subset_settings_menu->get_item_by_id(MNID_SUBSETSAVECHANGES).kind = MN_DEACTIVE;
        else
          subset_settings_menu->get_item_by_id(MNID_SUBSETSAVECHANGES).kind = MN_ACTION;

        switch (i = subset_settings_menu->check())
        {
        case MNID_SUBSETSAVECHANGES:
          save_subset_settings_menu();
          Menu::set_current(leveleditor_menu);
          break;
        }
      }
    }

    MouseCursor::current()->draw();

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

int le_load_level_subset(char *filename)
{
  le_level_subset->load(filename);
  leveleditor_menu->get_item_by_id(MNID_SUBSETSETTINGS).kind = MN_GOTO;
  le_level = 1;
  le_goto_level(1);

  //GameSession* session = new GameSession(datadir + "/levels/" + le_level_subset->name + "/level1.stl", 0, ST_GL_DEMO_GAME);

  Menu::set_current(NULL);

  return 0;
}

void le_init_menus()
{
  int i;

  leveleditor_menu = new Menu();
  subset_load_menu = new Menu();
  subset_new_menu  = new Menu();
  subset_settings_menu = new Menu();
  level_settings_menu  = new Menu();
  select_tilegroup_menu  = new Menu();
  select_objects_menu = new Menu();

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
    subset_load_menu->additem(MN_ACTION,level_subsets.item[i],0,0, i+1);
  }
  subset_load_menu->additem(MN_HL,"",0,0);
  subset_load_menu->additem(MN_BACK,"Back",0,0);

  subset_new_menu->additem(MN_LABEL,"New Level Subset",0,0);
  subset_new_menu->additem(MN_HL,"",0,0);
  subset_new_menu->additem(MN_TEXTFIELD,"Enter Name",0,0,MNID_SUBSETNAME);
  subset_new_menu->additem(MN_ACTION,"Create",0,0, MNID_CREATESUBSET);
  subset_new_menu->additem(MN_HL,"",0,0);
  subset_new_menu->additem(MN_BACK,"Back",0,0);

  subset_settings_menu->additem(MN_LABEL,"Level Subset Settings",0,0);
  subset_settings_menu->additem(MN_HL,"",0,0);
  subset_settings_menu->additem(MN_TEXTFIELD,"Title",0,0,MNID_SUBSETTITLE);
  subset_settings_menu->additem(MN_TEXTFIELD,"Description",0,0,MNID_SUBSETDESCRIPTION);
  subset_settings_menu->additem(MN_HL,"",0,0);
  subset_settings_menu->additem(MN_ACTION,"Save Changes",0,0,MNID_SUBSETSAVECHANGES);
  subset_settings_menu->additem(MN_HL,"",0,0);
  subset_settings_menu->additem(MN_BACK,"Back",0,0);

  level_settings_menu->arrange_left = true;
  level_settings_menu->additem(MN_LABEL,"Level Settings",0,0);
  level_settings_menu->additem(MN_HL,"",0,0);
  level_settings_menu->additem(MN_TEXTFIELD,   "Name    ",0,0,MNID_NAME);
  level_settings_menu->additem(MN_TEXTFIELD,   "Author  ",0,0,MNID_AUTHOR);
  level_settings_menu->additem(MN_STRINGSELECT,"Song    ",0,0,MNID_SONG);
  level_settings_menu->additem(MN_STRINGSELECT,"Bg-Image",0,0,MNID_BGIMG);
  level_settings_menu->additem(MN_STRINGSELECT,"Particle",0,0,MNID_PARTICLE);
  level_settings_menu->additem(MN_NUMFIELD,    "Length  ",0,0,MNID_LENGTH);
  level_settings_menu->additem(MN_NUMFIELD,    "Time    ",0,0,MNID_TIME);
  level_settings_menu->additem(MN_NUMFIELD,    "Gravity ",0,0,MNID_GRAVITY);
  level_settings_menu->additem(MN_NUMFIELD,    "Bg-Img-Speed",0,0,MNID_BGSPEED);
  level_settings_menu->additem(MN_NUMFIELD,    "Top Red     ",0,0,MNID_TopRed);
  level_settings_menu->additem(MN_NUMFIELD,    "Top Green   ",0,0,MNID_TopGreen);
  level_settings_menu->additem(MN_NUMFIELD,    "Top Blue    ",0,0,MNID_TopBlue);
  level_settings_menu->additem(MN_NUMFIELD,    "Bottom Red  ",0,0,MNID_BottomRed);
  level_settings_menu->additem(MN_NUMFIELD,    "Bottom Green",0,0,MNID_BottomGreen);
  level_settings_menu->additem(MN_NUMFIELD,    "Bottom Blue ",0,0,MNID_BottomBlue);
  level_settings_menu->additem(MN_HL,"",0,0);
  level_settings_menu->additem(MN_ACTION,"Apply Changes",0,0,MNID_APPLY);

  select_tilegroup_menu->arrange_left = true;
  select_tilegroup_menu->additem(MN_LABEL,"Tilegroup",0,0);
  select_tilegroup_menu->additem(MN_HL,"",0,0);
  std::set<TileGroup>* tilegroups = TileManager::tilegroups();
  int tileid = 1;
  for(std::set<TileGroup>::iterator it = tilegroups->begin();
        it != tilegroups->end(); ++it )
    {
      select_tilegroup_menu->additem(MN_ACTION, it->name, 0, 0, tileid);
      tileid++;
      tilegroups_map[(*it).name] = new ButtonPanel(screen->w - 64,96, 64, 318);
      i = 0;

      for(std::vector<int>::const_iterator sit = (*it).tiles.begin();
          sit != (*it).tiles.end(); ++sit, ++i)
      {
        std::string imagefile = "/images/tilesets/" ;
        bool only_editor_image = false;
        if(!TileManager::instance()->get(*sit)->filenames.empty())
        {
          imagefile += TileManager::instance()->get(*sit)->filenames[0];
        }
        else if(!TileManager::instance()->get(*sit)->editor_filenames.empty())
        {
          imagefile += TileManager::instance()->get(*sit)->editor_filenames[0];
          only_editor_image = true;
        }
        else
        {
          imagefile += "notile.png";
        }
        Button* button = new Button(imagefile, it->name, SDLKey(SDLK_a + i),
                                    0, 0, 32, 32);
        if(!only_editor_image)
          if(!TileManager::instance()->get(*sit)->editor_filenames.empty())
          {
            imagefile = "/images/tilesets/" + TileManager::instance()->get(*sit)->editor_filenames[0];
            button->add_icon(imagefile,32,32);
          }
        tilegroups_map[it->name]->additem(button, *sit);
      }
    }
  select_tilegroup_menu->additem(MN_HL,"",0,0);

  select_objects_menu->arrange_left = true;
  select_objects_menu->additem(MN_LABEL,"Objects",0,0);
  select_objects_menu->additem(MN_HL,"",0,0);
  select_objects_menu->additem(MN_ACTION,"BadGuys",0,0,1);
  objects_map["BadGuys"] = new ButtonPanel(screen->w - 64,96, 64, 318);

  for(int i = 0; i < NUM_BadGuyKinds; ++i)
  {
    BadGuy bad_tmp(0,0,BadGuyKind(i),false);
    objects_map["BadGuys"]->additem(new Button("", "BadGuy",(SDLKey)(i+'a'),0,0,32,32),1000000+i);
    objects_map["BadGuys"]->manipulate_button(i)->set_game_object(new BadGuy(objects_map["BadGuys"]->manipulate_button(i)->get_pos().x,objects_map["BadGuys"]->manipulate_button(i)->get_pos().y,BadGuyKind(i),false));
  }

  select_objects_menu->additem(MN_HL,"",0,0);

}

int le_init()
{


  level_subsets = dsubdirs("/levels", "level1.stl");
  le_level_subset = new LevelSubset;

  le_world = NULL;
  selected_game_object = NULL;

  active_tm = TM_IA;
  le_show_grid = true;
  show_selections = true;
  scroll_x = 0;

  fire = DOWN;
  done = 0;
  le_frame = 0;	/* support for frames in some tiles, like waves and bad guys */
  le_level_changed = false;
  le_help_shown = false;

  le_mouse_pressed[LEFT] = false;
  le_mouse_pressed[RIGHT] = false;

  le_mouse_clicked[LEFT] = false;
  le_mouse_clicked[RIGHT] = false;

  le_selection = new Surface(datadir + "/images/icons/select.png", USE_ALPHA);

  select_tilegroup_menu_effect.init(false);
  select_objects_menu_effect.init(false);
  display_level_info.init(false);

  /* Load buttons */
  le_save_level_bt = new Button("/images/icons/save.png","Save level", SDLK_F6,screen->w-64,32);
  le_exit_bt = new Button("/images/icons/exit.png","Exit", SDLK_F10,screen->w-32,32);
  le_next_level_bt = new Button("/images/icons/up.png","Next level", SDLK_PAGEUP,screen->w-64,0);
  le_previous_level_bt = new Button("/images/icons/down.png","Previous level",SDLK_PAGEDOWN,screen->w-32,0);
  le_rubber_bt = new Button("/images/icons/rubber.png","Rubber",SDLK_DELETE,screen->w-32,48);
  le_select_mode_one_bt = new Button ("/images/icons/select-mode1.png","Select single tile",SDLK_F3,screen->w-64,48);
  le_select_mode_two_bt = new Button("/images/icons/select-mode2.png","Select multiple tiles",SDLK_F3,screen->w-64,48);
  le_test_level_bt = new Button("/images/icons/test-level.png","Test level",SDLK_F4,screen->w-64,screen->h - 64);
  le_settings_bt = new Button("/images/icons/settings.png","Level settings",SDLK_F5,screen->w-32,screen->h - 64);
  le_move_left_bt = new Button("/images/icons/left.png","Move left",SDLK_LEFT,0,0);
  le_move_right_bt = new Button("/images/icons/right.png","Move right",SDLK_RIGHT,screen->w-80,0);
  le_tilegroup_bt = new Button("/images/icons/tilegroup.png","Select Tilegroup", SDLK_F7,screen->w-64,64);
  le_objects_bt = new Button("/images/icons/objects.png","Select Objects", SDLK_F8,screen->w-64,80);
  le_object_select_bt = new Button("/images/icons/select-one.png","Select an Object", SDLK_s, screen->w - 64, screen->h-98);
  le_object_properties_bt = new Button("/images/icons/properties.png","Edit object properties", SDLK_p, screen->w - 32, screen->h-98);
  le_object_properties_bt->set_active(false);

  mouse_select_object = new MouseCursor(datadir + "/images/status/select-cursor.png",1);
  mouse_select_object->set_mid(16,16);

  le_tilemap_panel = new ButtonPanel(screen->w-64,screen->h-32,32,32);
  le_tilemap_panel->set_button_size(32,10);
  le_tilemap_panel->additem(new Button("/images/icons/bkgrd.png","Background",SDLK_b,0,0),TM_BG);
  le_tilemap_panel->additem(new Button("/images/icons/intact.png","Interactive",SDLK_i,0,0),TM_IA);
  le_tilemap_panel->additem(new Button("/images/icons/frgrd.png","Foreground",SDLK_f,0,0),TM_FG);
  le_tilemap_panel->highlight_last(true);
  le_tilemap_panel->set_last_clicked(TM_IA);

  le_current.Init();

  le_init_menus();

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);


  return 0;
}

void update_level_settings_menu()
{
  char str[80];
  int i;

  level_settings_menu->get_item_by_id(MNID_NAME).change_input(le_world->get_level()->name.c_str());
  level_settings_menu->get_item_by_id(MNID_AUTHOR).change_input(le_world->get_level()->author.c_str());

  string_list_copy(level_settings_menu->get_item_by_id(MNID_SONG).list, dfiles("music/",NULL, "-fast"));
  string_list_copy(level_settings_menu->get_item_by_id(MNID_BGIMG).list, dfiles("images/background",NULL, NULL));
  string_list_add_item(level_settings_menu->get_item_by_id(MNID_BGIMG).list,"");
  string_list_add_item(level_settings_menu->get_item_by_id(MNID_PARTICLE).list,"");
  string_list_add_item(level_settings_menu->get_item_by_id(MNID_PARTICLE).list,"snow");
  string_list_add_item(level_settings_menu->get_item_by_id(MNID_PARTICLE).list,"clouds");

  if((i = string_list_find(level_settings_menu->get_item_by_id(MNID_SONG).list,le_world->get_level()->song_title.c_str())) != -1)
    level_settings_menu->get_item_by_id(MNID_SONG).list->active_item = i;
  if((i = string_list_find(level_settings_menu->get_item_by_id(MNID_BGIMG).list,le_world->get_level()->bkgd_image.c_str())) != -1)
    level_settings_menu->get_item_by_id(MNID_BGIMG).list->active_item = i;
  if((i = string_list_find(level_settings_menu->get_item_by_id(MNID_PARTICLE).list,le_world->get_level()->particle_system.c_str())) != -1)
    level_settings_menu->get_item_by_id(MNID_PARTICLE).list->active_item = i;

  sprintf(str,"%d",le_world->get_level()->width);
  level_settings_menu->get_item_by_id(MNID_LENGTH).change_input(str);
  sprintf(str,"%d",le_world->get_level()->time_left);
  level_settings_menu->get_item_by_id(MNID_TIME).change_input(str);
  sprintf(str,"%2.0f",le_world->get_level()->gravity);
  level_settings_menu->get_item_by_id(MNID_GRAVITY).change_input(str);
  sprintf(str,"%d",le_world->get_level()->bkgd_speed);
  level_settings_menu->get_item_by_id(MNID_BGSPEED).change_input(str);
  sprintf(str,"%d",le_world->get_level()->bkgd_top.red);
  level_settings_menu->get_item_by_id(MNID_TopRed).change_input(str);
  sprintf(str,"%d",le_world->get_level()->bkgd_top.green);
  level_settings_menu->get_item_by_id(MNID_TopGreen).change_input(str);
  sprintf(str,"%d",le_world->get_level()->bkgd_top.blue);
  level_settings_menu->get_item_by_id(MNID_TopBlue).change_input(str);
  sprintf(str,"%d",le_world->get_level()->bkgd_bottom.red);
  level_settings_menu->get_item_by_id(MNID_BottomRed).change_input(str);
  sprintf(str,"%d",le_world->get_level()->bkgd_bottom.green);
  level_settings_menu->get_item_by_id(MNID_BottomGreen).change_input(str);
  sprintf(str,"%d",le_world->get_level()->bkgd_bottom.blue);
  level_settings_menu->get_item_by_id(MNID_BottomBlue).change_input(str);
}

void update_subset_settings_menu()
{
  subset_settings_menu->item[2].change_input(le_level_subset->title.c_str());
  subset_settings_menu->item[3].change_input(le_level_subset->description.c_str());
}

void apply_level_settings_menu()
{
  int i;
  i = false;

  le_world->get_level()->name = level_settings_menu->get_item_by_id(MNID_NAME).input;
  le_world->get_level()->author = level_settings_menu->get_item_by_id(MNID_AUTHOR).input;

  if(le_world->get_level()->bkgd_image.compare(string_list_active(level_settings_menu->get_item_by_id(MNID_BGIMG).list)) != 0)
  {
    le_world->get_level()->bkgd_image = string_list_active(level_settings_menu->get_item_by_id(MNID_BGIMG).list);
    i = true;
  }

  if(le_world->get_level()->particle_system.compare(string_list_active(level_settings_menu->get_item_by_id(MNID_PARTICLE).list)) != 0)
  {
    le_world->get_level()->particle_system = string_list_active(level_settings_menu->get_item_by_id(MNID_PARTICLE).list);
  }

  if(i)
  {
    le_world->get_level()->load_gfx();
  }

  le_world->get_level()->song_title = string_list_active(level_settings_menu->get_item_by_id(MNID_SONG).list);

  le_world->get_level()->change_size(atoi(level_settings_menu->get_item_by_id(MNID_LENGTH).input));
  le_world->get_level()->time_left = atoi(level_settings_menu->get_item_by_id(MNID_TIME).input);
  le_world->get_level()->gravity = atof(level_settings_menu->get_item_by_id(MNID_GRAVITY).input);
  le_world->get_level()->bkgd_speed = atoi(level_settings_menu->get_item_by_id(MNID_BGSPEED).input);
  le_world->get_level()->bkgd_top.red = atoi(level_settings_menu->get_item_by_id(MNID_TopRed).input);
  le_world->get_level()->bkgd_top.green = atoi(level_settings_menu->get_item_by_id(MNID_TopGreen).input);
  le_world->get_level()->bkgd_top.blue = atoi(level_settings_menu->get_item_by_id(MNID_TopBlue).input);
  le_world->get_level()->bkgd_bottom.red = atoi(level_settings_menu->get_item_by_id(MNID_BottomRed).input);
  le_world->get_level()->bkgd_bottom.green = atoi(level_settings_menu->get_item_by_id(MNID_BottomGreen).input);
  le_world->get_level()->bkgd_bottom.blue = atoi(level_settings_menu->get_item_by_id(MNID_BottomBlue).input);
}

void save_subset_settings_menu()
{
  le_level_subset->title = subset_settings_menu->item[2].input;
  le_level_subset->description = subset_settings_menu->item[3].input;
  le_level_subset->save();
}

void le_unload_level()
{
  if(le_level_changed)
  {
    le_drawlevel();
    le_drawinterface();
    char str[1024];
    sprintf(str,"Save changes to level %d of %s?",le_level,le_level_subset->name.c_str());
    if(confirm_dialog(str))
    {
      le_world->get_level()->save(le_level_subset->name.c_str(),le_level);
    }
  }

  delete le_world;
  le_level_changed = false;
}

void le_goto_level(int levelnb)
{
  le_unload_level();
  le_world = new World(le_level_subset->name, levelnb);
  display_level_info.start(2500);
  le_level = levelnb;
}

void le_quit(void)
{
  SDL_EnableKeyRepeat(0, 0);    // disables key repeating

  le_unload_level();
  delete le_selection;
  delete leveleditor_menu;
  delete subset_load_menu;
  delete subset_new_menu;
  delete subset_settings_menu;
  delete level_settings_menu;
  delete select_tilegroup_menu;
  delete select_objects_menu;
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
  delete le_objects_bt;
  delete le_tilemap_panel;
  delete le_object_select_bt;
  delete le_object_properties_bt;
  delete mouse_select_object;

  delete le_level_subset;
  le_level_subset = 0;

  for(ButtonPanelMap::iterator i = tilegroups_map.begin();
      i != tilegroups_map.end(); ++i)
  {
    delete i->second;
  }
  for(ButtonPanelMap::iterator i = objects_map.begin();
      i != objects_map.end(); ++i)
  {
    delete i->second;
  }
}

void le_drawminimap()
{
  if(le_world == NULL)
    return;

  int mini_tile_width;
  if(screen->w - 64 > le_world->get_level()->width * 4)
    mini_tile_width = 4;
  else if(screen->w - 64 > le_world->get_level()->width * 2)
    mini_tile_width = 2;
  else
    mini_tile_width = 1;
  int left_offset = (screen->w - 64 - le_world->get_level()->width*mini_tile_width) / 2;

  for (int y = 0; y < 15; ++y)
    for (int x = 0; x < le_world->get_level()->width; ++x)
    {

      Tile::draw_stretched(left_offset + mini_tile_width*x, y * 4, mini_tile_width , 4, le_world->get_level()->bg_tiles[y][x]);

      Tile::draw_stretched(left_offset + mini_tile_width*x, y * 4, mini_tile_width , 4, le_world->get_level()->ia_tiles[y][x]);

      Tile::draw_stretched(left_offset + mini_tile_width*x, y * 4, mini_tile_width , 4, le_world->get_level()->fg_tiles[y][x]);

    }

  fillrect(left_offset, 0, le_world->get_level()->width*mini_tile_width, 15*4, 200, 200, 200, 128);

  fillrect(left_offset + (pos_x/32)*mini_tile_width, 0, 19*mini_tile_width, 1, 255, 255, 255, 255);
  fillrect(left_offset + (pos_x/32)*mini_tile_width, 0, 1, 15*4, 255, 255, 255, 255);
  fillrect(left_offset + (pos_x/32)*mini_tile_width + 19*mini_tile_width, 0, 1, 15*4, 255, 255, 255, 255);
  fillrect(left_offset + (pos_x/32)*mini_tile_width, 15*4-2, 19*mini_tile_width, 1, 255, 255, 255, 255);

}

void le_drawinterface()
{
  int x,y;
  char str[80];

  if(le_world != NULL)
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

  if(show_minimap)
    le_drawminimap();

  if(show_selections && MouseCursor::current() != mouse_select_object)
  {
    if(le_selection_mode == SM_CURSOR)
    {
      if(le_current.IsTile())
        le_selection->draw( cursor_x - pos_x, cursor_y);
    }
    else if(le_selection_mode == SM_SQUARE)
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
  }


  /* draw button bar */
  fillrect(screen->w - 64, 0, 64, screen->h, 50, 50, 50,255);

  if(le_current.IsTile())
  {
    Tile::draw(19 * 32, 14 * 32, le_current.tile);
    if(TileManager::instance()->get(le_current.tile)->editor_images.size() > 0)
      TileManager::instance()->get(le_current.tile)->editor_images[0]->draw( 19 * 32, 14 * 32);
  }
  if(le_current.IsObject() && MouseCursor::current() != mouse_select_object)
  {
    le_current.obj->draw_on_screen(19 * 32, 14 * 32);
    le_current.obj->draw_on_screen(cursor_x,cursor_y);
  }

  if(mouse_select_object && selected_game_object != NULL)
  {
    fillrect(selected_game_object->base.x-pos_x,selected_game_object->base.y,selected_game_object->base.width,3,255,0,0,255);
    fillrect(selected_game_object->base.x-pos_x,selected_game_object->base.y,3,selected_game_object->base.height,255,0,0,255);
    fillrect(selected_game_object->base.x-pos_x,selected_game_object->base.y+selected_game_object->base.height,selected_game_object->base.width,3,255,0,0,255);
    fillrect(selected_game_object->base.x-pos_x+selected_game_object->base.width,selected_game_object->base.y,3,selected_game_object->base.height,255,0,0,255);
  }

  if(le_world != NULL)
  {
    le_save_level_bt->draw();
    le_exit_bt->draw();
    le_test_level_bt->draw();
    le_next_level_bt->draw();
    le_previous_level_bt->draw();
    le_rubber_bt->draw();
    if(le_selection_mode == SM_SQUARE)
      le_select_mode_one_bt->draw();
    else if(le_selection_mode == SM_CURSOR)
      le_select_mode_two_bt->draw();
    le_settings_bt->draw();
    le_move_right_bt->draw();
    le_move_left_bt->draw();
    le_tilegroup_bt->draw();
    le_objects_bt->draw();
    if(!cur_tilegroup.empty())
      tilegroups_map[cur_tilegroup]->draw();
    else if(!cur_objects.empty())
    {
      objects_map[cur_objects]->draw();
    }

    le_tilemap_panel->draw();

    if(!cur_objects.empty())
    {
      le_object_select_bt->draw();
      le_object_properties_bt->draw();
    }

    sprintf(str, "%d/%d", le_level,le_level_subset->levels);
    white_text->drawf(str, (le_level_subset->levels < 10) ? -10 : 0, 16, A_RIGHT, A_TOP, 0);

    if(!le_help_shown)
      white_small_text->draw("F1 for Help", 10, 430, 1);

    if(display_level_info.check())
      white_text->drawf(le_world->get_level()->name.c_str(), 0, 0, A_HMIDDLE, A_TOP, 0);
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
  unsigned int y,x,s;
  Uint8 a;

  /* Draw the real background */
  if(le_world->get_level()->bkgd_image[0] != '\0')
  {
    s = (int)((float)pos_x * ((float)le_world->get_level()->bkgd_speed/100.0f)) % screen->w;
    le_world->get_level()->img_bkgd->draw_part(s,0,0,0,
        le_world->get_level()->img_bkgd->w - s - 32, le_world->get_level()->img_bkgd->h);
    le_world->get_level()->img_bkgd->draw_part(0,0,screen->w - s - 32 ,0,s,
        le_world->get_level()->img_bkgd->h);
  }
  else
  {
    drawgradient(le_world->get_level()->bkgd_top, le_world->get_level()->bkgd_bottom);
  }

  if(le_current.IsTile())
  {
    Tile::draw(cursor_x-pos_x, cursor_y,le_current.tile,128);
    if(!TileManager::instance()->get(le_current.tile)->images.empty())
      fillrect(cursor_x-pos_x,cursor_y,TileManager::instance()->get(le_current.tile)->images[0]->w,TileManager::instance()->get(le_current.tile)->images[0]->h,50,50,50,50);
  }
  if(le_current.IsObject())
  {
    le_current.obj->move_to(cursor_x, cursor_y);
  }

  /*       clearscreen(current_level.bkgd_red, current_level.bkgd_green, current_level.bkgd_blue); */

  for (y = 0; y < 15; ++y)
    for (x = 0; x < 20; ++x)
    {

      if(active_tm == TM_BG)
        a = 255;
      else
        a = 128;

      Tile::draw(32*x - fmodf(pos_x, 32), y * 32, le_world->get_level()->bg_tiles[y][x + (int)(pos_x / 32)],a);

      if(active_tm == TM_IA)
        a = 255;
      else
        a = 128;

      Tile::draw(32*x - fmodf(pos_x, 32), y * 32, le_world->get_level()->ia_tiles[y][x + (int)(pos_x / 32)],a);

      if(active_tm == TM_FG)
        a = 255;
      else
        a = 128;

      Tile::draw(32*x - fmodf(pos_x, 32), y * 32, le_world->get_level()->fg_tiles[y][x + (int)(pos_x / 32)],a);

      /* draw whats inside stuff when cursor is selecting those */
      /* (draw them all the time - is this the right behaviour?) */
      Tile* edit_image = TileManager::instance()->get(le_world->get_level()->ia_tiles[y][x + (int)(pos_x / 32)]);
      if(edit_image && !edit_image->editor_images.empty())
        edit_image->editor_images[0]->draw( x * 32 - ((int)pos_x % 32), y*32);

    }

  /* Draw the Bad guys: */
  for (std::list<BadGuy*>::iterator it = le_world->bad_guys.begin(); it != le_world->bad_guys.end(); ++it)
  {
    /* to support frames: img_bsod_left[(frame / 5) % 4] */

    scroll_x = pos_x;
    (*it)->draw();
  }


  /* Draw the player: */
  /* for now, the position is fixed at (100, 240) */
  largetux.walk_right->draw( 100 - pos_x, 240);
}

void le_change_object_properties(GameObject *pobj)
{
  Surface* cap_screen = Surface::CaptureScreen();
  Menu* object_properties_menu = new Menu();
  bool loop = true;

  object_properties_menu->additem(MN_LABEL,pobj->type() + " Properties",0,0);
  object_properties_menu->additem(MN_HL,"",0,0);

  if(pobj->type() == "BadGuy")
  {
    BadGuy* pbad = dynamic_cast<BadGuy*>(pobj);
    object_properties_menu->additem(MN_STRINGSELECT,"Kind",0,0,1);
    for(int i = 0; i < NUM_BadGuyKinds; ++i)
    {
      string_list_add_item(object_properties_menu->get_item_by_id(1).list,badguykind_to_string(static_cast<BadGuyKind>(i)).c_str());
      if(pbad->kind == i)
        object_properties_menu->get_item_by_id(1).list->active_item = i;
    }
    object_properties_menu->additem(MN_TOGGLE,"StayOnPlatform",pbad->stay_on_platform,0,2);
  }

  object_properties_menu->additem(MN_HL,"",0,0);
  object_properties_menu->additem(MN_ACTION,"Ok",0,0,3);

  Menu::set_current(object_properties_menu);

  while(loop)
  {
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
      object_properties_menu->event(event);
    }

    cap_screen->draw(0,0);

    object_properties_menu->draw();
    object_properties_menu->action();

    switch (object_properties_menu->check())
    {
    case 3:
      if(pobj->type() == "BadGuy")
      {
        BadGuy* pbad = dynamic_cast<BadGuy*>(pobj);
        pbad->kind =  badguykind_from_string(string_list_active(object_properties_menu->get_item_by_id(1).list));
        pbad->stay_on_platform = object_properties_menu->get_item_by_id(2).toggled;
	int i = 0;
	std::list<BadGuy*>::iterator it;
        for(it = le_world->bad_guys.begin(); it != le_world->bad_guys.end(); ++it, ++i)
          if((*it) == pbad)
            break;
        le_world->get_level()->badguy_data[i].kind = pbad->kind;
	le_world->get_level()->badguy_data[i].stay_on_platform = pbad->stay_on_platform;
	delete (*it);
	(*it) = new BadGuy(le_world->get_level()->badguy_data[i].x,le_world->get_level()->badguy_data[i].y,le_world->get_level()->badguy_data[i].kind,le_world->get_level()->badguy_data[i].stay_on_platform);
      }
      loop = false;
      break;
    default:
      break;
    }

    if(Menu::current() == NULL)
      loop = false;

    mouse_cursor->draw();
    flipscreen();
    SDL_Delay(25);
  }

  delete cap_screen;
  Menu::set_current(0);
  delete object_properties_menu;
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
      if(!le_world && !Menu::current())
        Menu::set_current(leveleditor_menu);
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
          case 265:
            Menu::set_current(leveleditor_menu);
            break;
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
            if(le_world != NULL)
              le_showhelp();
            break;
          case SDLK_HOME:
            cursor_x = 0;
            pos_x = cursor_x;
            break;
          case SDLK_END:
            cursor_x = (le_world->get_level()->width * 32) - 32;
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
          {
            le_mouse_pressed[LEFT] = false;
            le_mouse_clicked[LEFT] = true;
          }
          else if(event.button.button == SDL_BUTTON_RIGHT)
          {
            le_mouse_pressed[RIGHT] = false;
            le_mouse_clicked[RIGHT] = true;
          }
          break;
        case SDL_MOUSEMOTION:

          if(!Menu::current())
          {
            x = event.motion.x;
            y = event.motion.y;

            if(le_current.IsTile())
            {
              cursor_x = ((int)(pos_x + x) / 32) * 32;
              cursor_y = ((int) y / 32) * 32;
            }
            else
            {
              cursor_x = x;
              cursor_y = y;
            }

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
        default:
          break;
        }
      }
      else if(event.type == SDL_QUIT) /* window closing */
      {
      done = 1;
      }
    }

    if(le_world != NULL)
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
            le_world->get_level()->save(le_level_subset->name.c_str(),le_level);
          le_exit_bt->event(event);
          if(le_exit_bt->get_state() == BUTTON_CLICKED)
          {
            Menu::set_current(leveleditor_menu);
          }
          le_next_level_bt->event(event);
          if(le_next_level_bt->get_state() == BUTTON_CLICKED)
          {
            if(le_level < le_level_subset->levels)
            {
              le_goto_level(le_level+1);
            }
            else
            {
              Level new_lev;
              char str[1024];
              sprintf(str,"Level %d doesn't exist. Create it?",le_level+1);
              if(confirm_dialog(str))
              {
                new_lev.init_defaults();
                new_lev.save(le_level_subset->name.c_str(),le_level+1);
                le_level_subset->levels = le_level;
                le_goto_level(le_level);
              }
            }
          }
          le_previous_level_bt->event(event);
          if(le_previous_level_bt->get_state() == BUTTON_CLICKED)
          {
            if(le_level > 1)
              le_goto_level(le_level -1);
          }
          le_rubber_bt->event(event);
          if(le_rubber_bt->get_state() == BUTTON_CLICKED)
            le_current.Tile(0);

          if(!cur_objects.empty())
          {
            le_object_select_bt->event(event);
            if(le_object_select_bt->get_state() == BUTTON_CLICKED)
            {
              MouseCursor::set_current(mouse_select_object);
            }

            le_object_properties_bt->event(event);
            if(le_object_properties_bt->get_state() == BUTTON_CLICKED)
            {
              le_change_object_properties(selected_game_object);
            }
          }


          if(le_selection_mode == SM_SQUARE)
          {
            le_select_mode_one_bt->event(event);
            if(le_select_mode_one_bt->get_state() == BUTTON_CLICKED)
              le_selection_mode = SM_CURSOR;
          }
          else
          {
            le_select_mode_two_bt->event(event);
            if(le_select_mode_two_bt->get_state() == BUTTON_CLICKED)
              le_selection_mode = SM_SQUARE;
          }
          ButtonPanelMap::iterator it;
          le_tilegroup_bt->event(event);
          switch (le_tilegroup_bt->get_state())
          {
          case BUTTON_CLICKED:
            Menu::set_current(select_tilegroup_menu);
            select_tilegroup_menu_effect.start(200);
            select_tilegroup_menu->set_pos(screen->w - 64,100,-0.5,0.5);
            break;
          case BUTTON_WHEELUP:
            if(cur_tilegroup.empty())
            {
              cur_tilegroup = tilegroups_map.begin()->first;
            }
            else
            {
              it = tilegroups_map.find(cur_tilegroup);
              if((++it) == tilegroups_map.end())
              {
                cur_tilegroup = tilegroups_map.begin()->first;
              }
              else
              {
                cur_tilegroup = (*it).first;
              }
            }

            cur_objects = "";
            break;
          case BUTTON_WHEELDOWN:
            it = tilegroups_map.find(cur_tilegroup);
            if(it == tilegroups_map.begin())
            {
              cur_tilegroup = tilegroups_map.rbegin()->first;
              cur_objects = "";
              break;
            }
            if(--it != --tilegroups_map.begin())
              cur_tilegroup = (*it).first;
            else
              cur_tilegroup = tilegroups_map.rbegin()->first;

            cur_objects = "";
            break;
          default:
            break;
          }

          le_objects_bt->event(event);
          switch (le_objects_bt->get_state())
          {
          case BUTTON_CLICKED:
            Menu::set_current(select_objects_menu);
            select_objects_menu_effect.start(200);
            select_objects_menu->set_pos(screen->w - 64,100,-0.5,0.5);
            break;
          case BUTTON_WHEELUP:
            it = objects_map.find(cur_objects);
            if(it == objects_map.end())
            {
              cur_objects = objects_map.begin()->first;
              cur_tilegroup = "";
              break;
            }
            if(++it != objects_map.end())
              cur_objects = (*it).first;
            else
              cur_objects = objects_map.begin()->first;

            cur_tilegroup = "";
            break;
          case BUTTON_WHEELDOWN:
            it = objects_map.find(cur_objects);
            if(it == objects_map.begin())
            {
              cur_objects = objects_map.rbegin()->first;
              cur_tilegroup = "";
              break;
            }
            if(--it != --objects_map.begin())
              cur_objects = (*it).first;
            else
              cur_objects = objects_map.rbegin()->first;

            cur_tilegroup = "";
            break;
            break;
          default:
            break;
          }

          le_settings_bt->event(event);
          if(le_settings_bt->get_state() == BUTTON_CLICKED)
          {
            update_level_settings_menu();
            Menu::set_current(level_settings_menu);
          }
          if(!cur_tilegroup.empty())
          {
            if((pbutton = tilegroups_map[cur_tilegroup]->event(event)) != NULL)
            {
              if(pbutton->get_state() == BUTTON_CLICKED)
              {
                le_current.Tile(pbutton->get_tag());
              }
            }
          }
          else if(!cur_objects.empty())
          {
            if((pbutton = objects_map[cur_objects]->event(event)) != NULL)
            {
              if(pbutton->get_state() == BUTTON_CLICKED)
              {
                le_current.Object(pbutton->get_game_object());
              }
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
          le_objects_bt->event(event);
          if(le_objects_bt->get_state() == BUTTON_CLICKED)
          {
            Menu::set_current(0);
          }
        }
      }

      if(!Menu::current() && !show_minimap)
      {
        if(le_mouse_pressed[LEFT])
        {
          if(MouseCursor::current() != mouse_select_object)
          {
            if(le_current.IsTile())
              le_change(cursor_x, cursor_y, active_tm, le_current.tile);
          }
        }
        else if(le_mouse_clicked[LEFT])
        {
          if(MouseCursor::current() == mouse_select_object)
          {
            int i = 0;
            bool object_got_hit = false;
            base_type cursor_base;
	    if(le_current.IsTile())
	    {
            cursor_base.x = cursor_x;
            cursor_base.y = cursor_y;
	    }
	    else if(le_current.IsObject())
	    {
            cursor_base.x = cursor_x + pos_x;
            cursor_base.y = cursor_y + pos_x;	    
	    }
            cursor_base.width = 32;
            cursor_base.height = 32;

            for(std::list<BadGuy*>::iterator it = le_world->bad_guys.begin(); it != le_world->bad_guys.end(); ++it, ++i)
              if(rectcollision(cursor_base,(*it)->base))
              {
                selected_game_object = (*it);
                object_got_hit = true;
                break;
              }

            if(!object_got_hit)
            {
              selected_game_object = NULL;
              le_object_properties_bt->set_active(false);
            }
            else
              le_object_properties_bt->set_active(true);

            MouseCursor::set_current(mouse_cursor);

          }
          else
          {
            if(le_current.IsObject())
            {
              le_level_changed  = true;
              std::string type = le_current.obj->type();
              if(type == "BadGuy")
              {
                BadGuy* pbadguy = dynamic_cast<BadGuy*>(le_current.obj);

                le_world->bad_guys.push_back(new BadGuy(cursor_x+scroll_x, cursor_y,pbadguy->kind,false));
                le_world->get_level()->badguy_data.push_back(le_world->bad_guys.back());
              }
            }
          }
	  
          le_mouse_clicked[LEFT] = false;

        }
      }
    }
  }
  if(!Menu::current())
  {
    show_minimap = false;

    le_move_left_bt->event(event);
    le_move_right_bt->event(event);
    switch(le_move_left_bt->get_state())
    {
    case BUTTON_PRESSED:
      pos_x -= 192;
      show_minimap = true;
      break;
    case BUTTON_HOVER:
      pos_x -= 32;
      show_minimap = true;
      break;
    case BUTTON_CLICKED:
      show_minimap = true;
      break;
    default:
      break;
    }

    switch(le_move_right_bt->get_state())
    {
    case BUTTON_PRESSED:
      pos_x += 192;
      show_minimap = true;
      break;
    case BUTTON_HOVER:
      pos_x += 32;
      show_minimap = true;
      break;
    case BUTTON_CLICKED:
      show_minimap = true;
      break;
    default:
      break;
    }

  if(le_mouse_pressed[RIGHT])
    show_minimap = true;
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
  if(le_world != NULL)
  {
    int xx,yy;
    int x1, x2, y1, y2;
    unsigned int i = 0;

    le_level_changed = true;

    switch(le_selection_mode)
    {
    case SM_CURSOR:
      le_world->get_level()->change(x,y,tm,c);

      base_type cursor_base;
      cursor_base.x = x;
      cursor_base.y = y;
      cursor_base.width = 32;
      cursor_base.height = 32;

      /* if there is a bad guy over there, remove it */
      for(std::list<BadGuy*>::iterator it = le_world->bad_guys.begin(); it != le_world->bad_guys.end(); ++it, ++i)
        if(rectcollision(cursor_base,(*it)->base))
        {
          delete (*it);
          le_world->bad_guys.erase(it);
          le_world->get_level()->badguy_data.erase(le_world->get_level()->badguy_data.begin() + i);
          break;
        }

      break;
    case SM_SQUARE:
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
      for(std::list<BadGuy*>::iterator it = le_world->bad_guys.begin();
          it != le_world->bad_guys.end(); /* will be at end of loop */)
      {
        if((*it)->base.x/32 >= x1 && (*it)->base.x/32 <= x2
            && (*it)->base.y/32 >= y1 && (*it)->base.y/32 <= y2)
        {
          delete (*it);
          it = le_world->bad_guys.erase(it);
          le_world->get_level()->badguy_data.erase(le_world->get_level()->badguy_data.begin() + i);
          continue;
        }
        else
        {
          ++i;
          ++it;
        }
      }

      for(xx = x1; xx <= x2; xx++)
        for(yy = y1; yy <= y2; yy++)
        {
          le_world->get_level()->change(xx*32, yy*32, tm, c);

        }
      break;
    default:
      break;
    }
  }
}

void le_testlevel()
{
  //Make sure a time value is set when testing the level
  if(le_world->get_level()->time_left == 0)
    le_world->get_level()->time_left = 250;

  le_world->get_level()->save("test", le_level);

  GameSession session("test",le_level, ST_GL_TEST);
  session.run();
  player_status.reset();

#ifndef NOSOUND
  music_manager->halt_music();
#endif

  Menu::set_current(NULL);
  World::set_current(le_world);
}

void le_showhelp()
{
  bool tmp_show_grid = le_show_grid;
  int temp_le_selection_mode = le_selection_mode;
  le_selection_mode = SM_NONE;
  show_selections = true;
  le_show_grid = false;
  le_help_shown = true;

  drawgradient(Color(0,0,0), Color(255,255,255));
  le_drawinterface();

  SDL_Event event;
  unsigned int i, done_;
  const char *text[] = {

                   " - Supertux level editor tutorial - ",
                   "",
                   "To make your map, click the       ",
                   "tilegroup button and choose a     ",
                   "tilegroup.",
                   "Pick a tile and simply hold down  ",
                   "the left mouse button over the map",
                   "to \"paint\" your selection over",
                   "the screen.",
                   "",
                   "There are three layers for painting",
                   "tiles upon, Background layer,",
                   "the Interactive layer, and the",
                   "Foreground layer, which can be",
                   "toggled by the BkGrd, IntAct and",
                   "FrGrd buttons. The Foreground and",
                   "Background layers do not effect",
                   "Tux in the gameplay, but lie in",
                   "front of him or lie behind him in",
                   "his adventures.",
                 };

  const char *text2[] = {

                    " - Supertux level editor tutorial - ",
                    "",
                    "The tiles placed on",
                    "the Interactive layer are those",
                    "which actually effect Tux in the",
                    "game.",
                    "",
                    "Click the objects menu to put ",
                    "bad guys and other objects in the",
                    "game. Unlike placing tiles, you",
                    "cannot \"paint\" enemies. Click",
                    "them onto the screen one at a time.",
                    "",
                    "To change the settings of your",
                    "level, click the button with the",
                    "screwdriver and wrench. From here",
                    "you can change the background,",
                    "music, length of the level,",
                    "and more."
                  };

  const char *text3[] = {

                    " - Supertux level editor tutorial - ",
                    "",
                    "You may have more than one level.",
                    "Pressing the up and down buttons",
                    "above the button bar lets you",
                    "choose which one you are working on.",
                    "",
                    "If you would like to speed up your",
                    "level editing, a useful trick is",
                    "to learn the keyboard shortcuts.",
                    "They are easy to learn, just right-",
                    "click on the buttons.",
                    "",
                    "Have fun making levels! If you make",
                    "some good ones, send them to us on",
                    "the SuperTux mailing list!",
                    "- SuperTux team"
                  };



  blue_text->drawf("- Help -", 0, 30, A_HMIDDLE, A_TOP, 2);

  for(i = 0; i < sizeof(text)/sizeof(char *); i++)
    white_text->draw(text[i], 5, 80+(i*white_text->h), 1);

  gold_text->drawf("Press Anything to Continue - Page 1/3", 0, 0, A_LEFT, A_BOTTOM, 1);

  flipscreen();

  done_ = 0;

  while(done_ == 0)
  {
    done_ = wait_for_event(event);
    SDL_Delay(50);
  }

  drawgradient(Color(0,0,0), Color(255,255,255));
  le_drawinterface();


  blue_text->drawf("- Help -", 0, 30, A_HMIDDLE, A_TOP, 2);

  for(i = 0; i < sizeof(text2)/sizeof(char *); i++)
    white_text->draw(text2[i], 5, 80+(i*white_text->h), 1);

  gold_text->drawf("Press Anything to Continue - Page 2/3", 0, 0, A_LEFT, A_BOTTOM, 1);

  flipscreen();

  done_ = 0;

  while(done_ == 0)
  {
    done_ = wait_for_event(event);
    SDL_Delay(50);
  }

  drawgradient(Color(0,0,0), Color(255,255,255));
  le_drawinterface();


  blue_text->drawf("- Help -", 0, 30, A_HMIDDLE, A_TOP, 2);

  for(i = 0; i < sizeof(text3)/sizeof(char *); i++)
    white_text->draw(text3[i], 5, 80+(i*white_text->h), 1);

  gold_text->drawf("Press Anything to Continue - Page 3/3", 0, 0, A_LEFT, A_BOTTOM, 1);

  flipscreen();

  done_ = 0;

  while(done_ == 0)
  {
    done_ = wait_for_event(event);
    SDL_Delay(50);
  }

  show_selections = true;
  le_show_grid = tmp_show_grid;
  le_selection_mode = temp_le_selection_mode;
  le_help_shown = false;
}
