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
#include <typeinfo>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cerrno>
#include <algorithm>
#include <iostream>
#include <unistd.h>

#include "SDL.h"
#include "SDL_image.h"

#include "leveleditor.h"
#include "video/screen.h"
#include "app/defines.h"
#include "app/globals.h"
#include "app/setup.h"
#include "sector.h"
#include "tilemap.h"
#include "gameloop.h"
#include "badguy.h"
#include "player.h"
#include "scene.h"
#include "tile.h"
#include "tile_manager.h"
#include "resources.h"
#include "background.h"
#include "camera.h"

/* definitions to aid development */

/* definitions that affect gameplay */
#define KEY_CURSOR_SPEED 32
#define KEY_CURSOR_FASTSPEED 64

/* when pagedown/up pressed speed:*/
#define PAGE_CURSOR_SPEED 13*32

#define MOUSE_LEFT_MARGIN 80
#define MOUSE_RIGHT_MARGIN (560-32)

/* scolling speed */
#define KEYBOARD_SPEED 140
#define MOUSE_SPEED    40

/* look */
#define SELECT_W 2 // size of the selections lines
#define SELECT_CLR 0, 255, 0, 255  // lines color (R, G, B, A)

/* Frames per second: */

#define FPS (1000 / 25)

enum { TM_IA, TM_BG, TM_FG };

LevelEditor::LevelEditor()
{
  level_subsets = dsubdirs("/levels", "level1.stl");
  le_level_subset = new LevelSubset;

  le_level = NULL;
  le_levelnb = 1;
  selected_game_object = NULL;

  active_tm = TM_IA;
  le_show_grid = true;
  show_selections = true;

  pos_x = pos_y = 0;

  done = 0;
  le_frame = 0;	/* support for frames in some tiles, like waves and bad guys */
  le_level_changed = false;
  le_help_shown = false;

  le_mouse_pressed[LEFT] = false;
  le_mouse_pressed[RIGHT] = false;

  le_mouse_clicked[LEFT] = false;
  le_mouse_clicked[RIGHT] = false;

  le_selection = new Surface(datadir + "/images/leveleditor/select.png", true);

  select_tilegroup_menu_effect.init(false);
  select_objects_menu_effect.init(false);
  display_level_info.init(false);

  /* Load buttons */
  le_save_level_bt = new Button("/images/icons/save.png","Save level", SDLK_F6,screen->w-64,32);
  le_exit_bt = new Button("/images/icons/exit.png","Exit", SDLK_F10,screen->w-32,32);
  le_next_level_bt = new Button("/images/icons/next.png","Next level", SDLK_PAGEUP,screen->w-64,0);
  le_previous_level_bt = new Button("/images/icons/previous.png","Previous level",SDLK_PAGEDOWN,screen->w-32,0);
  le_rubber_bt = new Button("/images/icons/rubber.png","Rubber",SDLK_DELETE,screen->w-32,48);
  le_select_mode_one_bt = new Button ("/images/icons/select-mode1.png","Select single tile",SDLK_F3,screen->w-64,48);
  le_select_mode_two_bt = new Button("/images/icons/select-mode2.png","Select multiple tiles",SDLK_F3,screen->w-64,48);
  le_test_level_bt = new Button("/images/icons/test-level.png","Test level",SDLK_F4,screen->w-64,screen->h - 64);
  le_settings_bt = new Button("/images/icons/settings.png","Level settings",SDLK_F5,screen->w-32,screen->h - 64);
  le_move_left_bt = new Button("/images/icons/left.png","Move left",SDLK_LEFT,screen->w-80-16,0);
  le_move_right_bt = new Button("/images/icons/right.png","Move right",SDLK_RIGHT,screen->w-80,0);
  le_move_up_bt = new Button("/images/icons/up.png","Move up",SDLK_UP,screen->w-80,16);
  le_move_down_bt = new Button("/images/icons/down.png","Move down",SDLK_DOWN,screen->w-80,32);
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
  le_tilemap_panel->additem(new Button("/images/icons/intact.png","Interactive",SDLK_i,0,0), TM_IA);
  le_tilemap_panel->additem(new Button("/images/icons/frgrd.png","Foreground",SDLK_f,0,0),TM_FG);
  le_tilemap_panel->highlight_last(true);
  le_tilemap_panel->set_last_clicked(TM_IA);

  le_current.Init();

  init_menus();

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}

LevelEditor::~LevelEditor()
{
  SDL_EnableKeyRepeat(0, 0);    // disables key repeating

  unload_level();
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
  delete le_move_up_bt;
  delete le_move_down_bt;
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

int LevelEditor::run(char* filename)
{
  int last_time, now_time, i;
  DrawingContext context;

  le_level = NULL;
  le_levelnb = 1;

  sound_manager->halt_music();

  while (SDL_PollEvent(&event))
  {}

  if(filename != NULL)
    if(load_level_subset(filename))
      return 1;

  while(true)
  {
    last_time = SDL_GetTicks();
    le_frame++;

    checkevents();

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

    if(le_level != NULL)
    {
      /* making events results to be in order */

      /* draw the level */
      context.set_translation(Vector(pos_x, pos_y));
      drawlevel(context);
    }
    else
      fillrect(0, 0, screen->w, screen->h, 0, 0, 0);

    /* draw editor interface */
    context.set_translation(Vector(0, 0));
    drawinterface(context);

    Menu* menu = Menu::current();
    if(menu)
    {
      menu->draw(context);
      menu->action();

      if(menu == leveleditor_menu)
      {
        switch (leveleditor_menu->check())
        {
        case MNID_RETURNLEVELEDITOR:
          if(le_level != NULL)
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
            if(load_level_subset(level_subsets.item[i-1]))
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
            goto_level(1);
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

    MouseCursor::current()->draw(context);

    if(done)
    {
      return 0;
    }

    ++global_frame_counter;

    SDL_Delay(25);
    now_time = SDL_GetTicks();
    if (now_time < last_time + FPS)
      SDL_Delay(last_time + FPS - now_time);	/* delay some time */

    context.do_drawing();
  }

  return done;
}

int LevelEditor::load_level_subset(char *filename)
{
  le_level_subset->load(filename);
  leveleditor_menu->get_item_by_id(MNID_SUBSETSETTINGS).kind = MN_GOTO;
  le_levelnb = 1;
  goto_level(le_levelnb);

  //GameSession* session = new GameSession(datadir + "/levels/" + le_level_subset->name + "/level1.stl", 0, ST_GL_DEMO_GAME);

  Menu::set_current(NULL);

  return 0;
}

void LevelEditor::init_menus()
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
  level_settings_menu->additem(MN_NUMFIELD,    "Height  ",0,0,MNID_HEIGHT);
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
        Tile& tile = TileManager::instance()->get(*sit);
        Surface* image;
        if(tile.editor_images.size() > 0)
          image = tile.editor_images[0];
        else if(tile.images.size() > 0)
          image = tile.images[0];
        else
          // TODO use some notile image...
          image = 0;

        Button* button = new Button(image, it->name, SDLKey(SDLK_a + i),
                                    0, 0, 32, 32);
        tilegroups_map[it->name]->additem(button, *sit);
      }
    }
  select_tilegroup_menu->additem(MN_HL,"",0,0);

  select_objects_menu->arrange_left = true;
  select_objects_menu->additem(MN_LABEL,"Objects",0,0);
  select_objects_menu->additem(MN_HL,"",0,0);
  // TODO fix this
  select_objects_menu->additem(MN_ACTION,"BadGuys",0,0,1);
  objects_map["BadGuys"] = new ButtonPanel(screen->w - 64,96, 64, 318);

  for(int i = 0; i < NUM_BadGuyKinds; ++i)
  {
//    BadGuy bad_tmp(BadGuyKind(i), 0, 0);
//    objects_map["BadGuys"]->additem(new Button(0, "BadGuy",(SDLKey)(i+'a'),0,0,32,32),1000000+i);
/* FIXME: maybe addbutton should already have a parameter for the surface
    objects_map["BadGuys"]->manipulate_button(i)->set_drawable(new
        BadGuy(BadGuyKind(i),
          objects_map["BadGuys"]->manipulate_button(i)->get_pos().x,
          objects_map["BadGuys"]->manipulate_button(i)->get_pos().y
          ));*/
  }

  select_objects_menu->additem(MN_HL,"",0,0);

}

void LevelEditor::update_level_settings_menu()
{
  char str[80];
  int i;

  level_settings_menu->get_item_by_id(MNID_NAME).change_input(le_level->name.c_str());
  level_settings_menu->get_item_by_id(MNID_AUTHOR).change_input(le_level->author.c_str());

  string_list_copy(level_settings_menu->get_item_by_id(MNID_SONG).list, dfiles("music/",NULL, "-fast"));
  string_list_copy(level_settings_menu->get_item_by_id(MNID_BGIMG).list, dfiles("images/background",NULL, NULL));
  string_list_add_item(level_settings_menu->get_item_by_id(MNID_BGIMG).list,"");
  string_list_add_item(level_settings_menu->get_item_by_id(MNID_PARTICLE).list,"");
  string_list_add_item(level_settings_menu->get_item_by_id(MNID_PARTICLE).list,"snow");
  string_list_add_item(level_settings_menu->get_item_by_id(MNID_PARTICLE).list,"clouds");

  if((i = string_list_find(level_settings_menu->get_item_by_id(MNID_SONG).list,le_level->get_sector("main")->song_title.c_str())) != -1)
    level_settings_menu->get_item_by_id(MNID_SONG).list->active_item = i;
  if((i = string_list_find(level_settings_menu->get_item_by_id(MNID_BGIMG).list,le_level->get_sector("main")->background->get_image().c_str())) != -1)
    level_settings_menu->get_item_by_id(MNID_BGIMG).list->active_item = i;
/*  if((i = string_list_find(level_settings_menu->get_item_by_id(MNID_PARTICLE).list,le_level->get_sector("main")->particlesystem.c_str())) != -1)
    level_settings_menu->get_item_by_id(MNID_PARTICLE).list->active_item = i;*/

  sprintf(str,"%d",static_cast<int>(le_level->get_sector("main")->solids->get_width()));
  level_settings_menu->get_item_by_id(MNID_LENGTH).change_input(str);
  sprintf(str,"%d", static_cast<int>(le_level->get_sector("main")->solids->get_height()));
  level_settings_menu->get_item_by_id(MNID_HEIGHT).change_input(str);
  sprintf(str,"%d",le_level->time_left);
  level_settings_menu->get_item_by_id(MNID_TIME).change_input(str);
  sprintf(str,"%2.0f",le_level->get_sector("main")->gravity);
  level_settings_menu->get_item_by_id(MNID_GRAVITY).change_input(str);
  sprintf(str,"%2.2f", le_level->get_sector("main")->background->get_speed());
  level_settings_menu->get_item_by_id(MNID_BGSPEED).change_input(str);
  sprintf(str,"%d",le_level->get_sector("main")->background->get_gradient_top().red);
  level_settings_menu->get_item_by_id(MNID_TopRed).change_input(str);
  sprintf(str,"%d",le_level->get_sector("main")->background->get_gradient_top().green);
  level_settings_menu->get_item_by_id(MNID_TopGreen).change_input(str);
  sprintf(str,"%d",le_level->get_sector("main")->background->get_gradient_top().blue);
  level_settings_menu->get_item_by_id(MNID_TopBlue).change_input(str);
  sprintf(str,"%d",le_level->get_sector("main")->background->get_gradient_bottom().red);
  level_settings_menu->get_item_by_id(MNID_BottomRed).change_input(str);
  sprintf(str,"%d",le_level->get_sector("main")->background->get_gradient_bottom().green);
  level_settings_menu->get_item_by_id(MNID_BottomGreen).change_input(str);
  sprintf(str,"%d",le_level->get_sector("main")->background->get_gradient_bottom().blue);
  level_settings_menu->get_item_by_id(MNID_BottomBlue).change_input(str);
}

void LevelEditor::update_subset_settings_menu()
{
  subset_settings_menu->item[2].change_input(le_level_subset->title.c_str());
  subset_settings_menu->item[3].change_input(le_level_subset->description.c_str());
}

void LevelEditor::apply_level_settings_menu()
{
  int i;
  i = false;
  le_level_changed = true;

  le_level->name = level_settings_menu->get_item_by_id(MNID_NAME).input;
  le_level->author = level_settings_menu->get_item_by_id(MNID_AUTHOR).input;

  if(le_level->get_sector("main")->background->get_image().compare(string_list_active(level_settings_menu->get_item_by_id(MNID_BGIMG).list)) != 0)
  {
    le_level->get_sector("main")->background->set_image(string_list_active(level_settings_menu->get_item_by_id(MNID_BGIMG).list), atoi(level_settings_menu->get_item_by_id(MNID_BGSPEED).input));
    i = true;
  }

/*  if(le_level->get_sector("main")->particlesystem.compare(string_list_active(level_settings_menu->get_item_by_id(MNID_PARTICLE).list)) != 0)
  {
    le_level->->get_sector("main")->particlesystem = string_list_active(level_settings_menu->get_item_by_id(MNID_PARTICLE).list);
  }*/

/*  if(i)
  {
    le_level->load_gfx();
  }*/

  le_level->get_sector("main")->song_title = string_list_active(level_settings_menu->get_item_by_id(MNID_SONG).list);

  le_level->get_sector("main")->solids->resize(
      atoi(level_settings_menu->get_item_by_id(MNID_LENGTH).input),
      atoi(level_settings_menu->get_item_by_id(MNID_HEIGHT).input));
  le_level->time_left = atoi(level_settings_menu->get_item_by_id(MNID_TIME).input);
  le_level->get_sector("main")->gravity = atof(level_settings_menu->get_item_by_id(MNID_GRAVITY).input);
  le_level->get_sector("main")->background->set_gradient(Color(
      atoi(level_settings_menu->get_item_by_id(MNID_TopRed).input),
      atoi(level_settings_menu->get_item_by_id(MNID_TopGreen).input),
      atoi(level_settings_menu->get_item_by_id(MNID_TopBlue).input)), Color(
      atoi(level_settings_menu->get_item_by_id(MNID_BottomRed).input),
      atoi(level_settings_menu->get_item_by_id(MNID_BottomGreen).input),
      atoi(level_settings_menu->get_item_by_id(MNID_BottomBlue).input)));
}

void LevelEditor::save_subset_settings_menu()
{
  le_level_subset->title = subset_settings_menu->item[2].input;
  le_level_subset->description = subset_settings_menu->item[3].input;
  le_level_subset->save();
  le_level_changed = false;
}

void LevelEditor::unload_level()
{
  if(le_level_changed)
  {
    char str[1024];
    // TODO get level number
    sprintf(str,"Save changes to level %d of %s?", 0/*le_level*/,le_level_subset->name.c_str());
    Surface* surf = new Surface(le_level->get_sector("main")->background->get_image(), false);
    if(confirm_dialog(surf, str))
    {
      le_level->save(le_level_subset->get_level_filename(le_levelnb));
    }
    if(surf != NULL)
      delete surf;
  }

  delete le_level;
  le_level_changed = false;
}

void LevelEditor::goto_level(int levelnb)
{
  unload_level();
  le_level = new Level();
  le_level->load(le_level_subset->get_level_filename(levelnb));
  display_level_info.start(2500);
  le_levelnb = levelnb;
}

void LevelEditor::drawminimap()
{
#if 0
//  if(le_level == NULL)
//    return;

  int mini_tile_width;
  if((unsigned)screen->w - 64 > le_level->get_sector("main")->solids->get_width() * 4)
    mini_tile_width = 4;
  else if((unsigned)screen->w - 64 > le_level->get_sector("main")->solids->get_width() * 2)
    mini_tile_width = 2;
  else
    mini_tile_width = 1;
  int left_offset = (screen->w - 64 - le_level->get_sector("main")->solids->get_width()*mini_tile_width) / 2;

  int mini_tile_height;
  if((unsigned)screen->h > le_level->get_sector("main")->solids->get_height() * 4)
    mini_tile_height = 4;
  else if((unsigned)screen->h > le_level->get_sector("main")->solids->get_height() * 2)
    mini_tile_height = 2;
  else
    mini_tile_height = 1;

  for (unsigned int y = 0; y < le_level->get_sector("main")->solids->get_height(); ++y)
    for (unsigned int x = 0; x < le_level->get_sector("main")->solids->get_width(); ++x)
    {

      Tile::draw_stretched(left_offset + mini_tile_width*x, y * mini_tile_height,
          mini_tile_width , mini_tile_height, le_level->bg_tiles[y * le_level->get_sector("main")->solids->get_width() + x]);

      Tile::draw_stretched(left_offset + mini_tile_width*x, y * mini_tile_height,
          mini_tile_width , mini_tile_height, le_level->ia_tiles[y * le_level->get_sector("main")->solids->get_width() + x]);

      Tile::draw_stretched(left_offset + mini_tile_width*x, y * mini_tile_height,
          mini_tile_width , mini_tile_height, le_level->fg_tiles[y + le_level->get_sector("main")->solids->get_width() + x]);

    }

  fillrect(left_offset, 0,
             le_level->get_sector("main")->solids->get_width()*mini_tile_width, le_level->get_sector("main")->solids->get_height()*mini_tile_height,
             200, 200, 200, 96);

  fillrect(left_offset + (pos_x/32)*mini_tile_width, (pos_y/32)*mini_tile_height,
             (VISIBLE_TILES_X-3)*mini_tile_width, 2,
             200, 200, 200, 200);
  fillrect(left_offset + (pos_x/32)*mini_tile_width, (pos_y/32)*mini_tile_height,
             2, (VISIBLE_TILES_Y-1)*mini_tile_height,
             200, 200, 200, 200);
  fillrect(left_offset + (pos_x/32)*mini_tile_width + (VISIBLE_TILES_X-3)*mini_tile_width - 2, (pos_y/32)*mini_tile_height,
             2, (VISIBLE_TILES_Y-1)*mini_tile_height,
             200, 200, 200, 200);
  fillrect(left_offset + (pos_x/32)*mini_tile_width, (pos_y/32)*mini_tile_height + (VISIBLE_TILES_Y-1)*mini_tile_height - 2,
             (VISIBLE_TILES_X-3)*mini_tile_width, 2,
             200, 200, 200, 200);
#endif
}

void LevelEditor::drawinterface(DrawingContext &context)
{
  int x,y;
  char str[80];

  if(le_level != NULL)
  {
    /* draw a grid (if selected) */
    if(le_show_grid)
    {
      for(x = 0; x < VISIBLE_TILES_X; x++)
        fillrect(x*32 - ((int)pos_x % 32), 0, 1, screen->h, 225, 225, 225,255);
      for(y = 0; y < VISIBLE_TILES_Y; y++)
        fillrect(0, y*32 - ((int)pos_y % 32), screen->w, 1, 225, 225, 225,255);
    }
  }

  if(show_minimap) // use_gl because the minimap isn't shown correctly in software mode. Any idea? FIXME Possible reasons: SDL_SoftStretch is a hack itsself || an alpha blitting issue SDL can't handle in software mode
    drawminimap();

  if(show_selections && MouseCursor::current() != mouse_select_object)
  {
    if(le_selection_mode == CURSOR)
    {
      if(le_current.IsTile())
        context.draw_surface(le_selection, Vector(cursor_x - pos_x, cursor_y - pos_y), LAYER_GUI);
    }
    else if(le_selection_mode == SQUARE)
    {
      int w, h;
      highlight_selection();
      /* draw current selection */
      w = selection.x2 - selection.x1;
      h = selection.y2 - selection.y1;
      context.draw_filled_rect(Vector(selection.x1 - pos_x, selection.y1 - pos_y), Vector(w, SELECT_W), Color(SELECT_CLR), LAYER_GUI);
      context.draw_filled_rect(Vector(selection.x1 - pos_x + w, selection.y1 - pos_y), Vector(SELECT_W, h), Color(SELECT_CLR), LAYER_GUI);
      context.draw_filled_rect(Vector(selection.x1 - pos_x, selection.y1 - pos_y + h), Vector(w, SELECT_W), Color(SELECT_CLR), LAYER_GUI);
      context.draw_filled_rect(Vector(selection.x1 - pos_x, selection.y1 - pos_y), Vector(SELECT_W, h), Color(SELECT_CLR), LAYER_GUI);
    }
  }


  /* draw button bar */
  context.draw_filled_rect(Vector(screen->w - 64, 0), Vector(64, screen->h), Color(50, 50, 50,255), LAYER_GUI);

  if(le_current.IsTile())
  {
//    le_level->get_sector("main")->solids->draw(context);

/*screen->w - 32, screen->h - 32, le_current.tile);
    if(TileManager::instance()->get(le_current.tile)->editor_images.size() > 0)
      TileManager::instance()->get(le_current.tile)->editor_images[0]->draw( screen->w - 32, screen->h - 32);*/
  }
#if 0 // XXX FIXME TODO: Do we have a new solution for draw_on_screen()?
  if(le_current.IsObject() && MouseCursor::current() != mouse_select_object)
  {
    le_current.obj->draw_on_screen(screen->w - 32, screen->h - 32);
    le_current.obj->draw_on_screen(cursor_x,cursor_y);
  }
#endif

  if(mouse_select_object && selected_game_object != NULL)
  {
    fillrect(selected_game_object->base.x-pos_x,selected_game_object->base.y-pos_y,selected_game_object->base.width,3,255,0,0,255);
    fillrect(selected_game_object->base.x-pos_x,selected_game_object->base.y-pos_y,3,selected_game_object->base.height,255,0,0,255);
    fillrect(selected_game_object->base.x-pos_x,selected_game_object->base.y-pos_y+selected_game_object->base.height,selected_game_object->base.width,3,255,0,0,255);
    fillrect(selected_game_object->base.x-pos_x+selected_game_object->base.width,selected_game_object->base.y-pos_y,3,selected_game_object->base.height,255,0,0,255);
  }

  if(le_level != NULL)
  {
    le_save_level_bt->draw(context);
    le_exit_bt->draw(context);
    le_test_level_bt->draw(context);
    le_next_level_bt->draw(context);
    le_previous_level_bt->draw(context);
    le_rubber_bt->draw(context);
    if(le_selection_mode == SQUARE)
      le_select_mode_one_bt->draw(context);
    else if(le_selection_mode == CURSOR)
      le_select_mode_two_bt->draw(context);
    le_settings_bt->draw(context);
    le_move_right_bt->draw(context);
    le_move_left_bt->draw(context);
    le_move_up_bt->draw(context);
    le_move_down_bt->draw(context);
    le_tilegroup_bt->draw(context);
    le_objects_bt->draw(context);
    if(!cur_tilegroup.empty())
      tilegroups_map[cur_tilegroup]->draw(context);
    else if(!cur_objects.empty())
    {
      objects_map[cur_objects]->draw(context);
    }

    le_tilemap_panel->draw(context);

    if(!cur_objects.empty())
    {
      le_object_select_bt->draw(context);
      le_object_properties_bt->draw(context);
    }

    sprintf(str, "%d/%d", le_levelnb, le_level_subset->get_num_levels());
    context.draw_text(white_text, str, Vector((le_level_subset->get_num_levels() < 10) ? -10 : 0, 16), LAYER_GUI);

    if(!le_help_shown)
      context.draw_text(white_small_text, "F1 for Help", Vector(10, 430), LAYER_GUI);

    if(display_level_info.check())
      context.draw_text_center(white_text, le_level->name.c_str(), Vector(0, 0), LAYER_GUI);
  }
  else
  {
    if(!Menu::current())
      context.draw_text(white_small_text, "No Level Subset loaded - Press ESC and choose one in the menu", Vector(10, 430), LAYER_GUI);
    else
      context.draw_text(white_small_text, "No Level Subset loaded", Vector(10, 430), LAYER_GUI);
  }

}

void LevelEditor::drawlevel(DrawingContext& context)
{
//  unsigned int y,x;
//  Uint8 a;

  /* Draw the real background */
  le_level->get_sector("main")->background->draw(context);

  if(le_current.IsTile())
  {
//le_level->get_sector("main")->solids->draw(context);
/*
    Tile::draw(cursor_x-pos_x, cursor_y-pos_y,le_current.tile,128);
    if(!TileManager::instance()->get(le_current.tile)->images.empty())
      fillrect(cursor_x-pos_x,cursor_y-pos_y,TileManager::instance()->get(le_current.tile)->images[0]->w,TileManager::instance()->get(le_current.tile)->images[0]->h,50,50,50,50);*/
  }
#if 0 // XXX FIXME TODO: Do we have a new solution for move_to()?
  if(le_current.IsObject())
  {
    le_current.obj->move_to(cursor_x, cursor_y);
  }
#endif

  /*       clearscreen(current_level.bkgd_red, current_level.bkgd_green, current_level.bkgd_blue); */

le_level->get_sector("main")->solids->draw(context);

// FIXME: make tiles to be drawn semi-transparent when not selected
#if 0
  for (y = 0; y < VISIBLE_TILES_Y && y < (unsigned)le_level->get_section("main")->height; ++y)
    for (x = 0; x < (unsigned)VISIBLE_TILES_X - 2; ++x)
    {

      if(active_tm == TM_BG)
        a = 255;
      else
        a = 128;

      Tile::draw(32*x - fmodf(pos_x, 32), y*32 - fmodf(pos_y, 32),
          le_level->bg_tiles[ (y + (int)(pos_y / 32)) * le_level->get_sector("main")->solids->get_width() + 
          (x + (int)(pos_x / 32))],a);

      if(active_tm == TM_IA)
        a = 255;
      else
        a = 128;

      Tile::draw(32*x - fmodf(pos_x, 32), y*32 - fmodf(pos_y, 32),
          le_level->ia_tiles[ (y + (int)(pos_y / 32)) * le_level->get_section("main")->width +       
          (x + (int)(pos_x / 32))],a);

      
      if(active_tm == TM_FG)
        a = 255;
      else
        a = 128;

      Tile::draw(32*x - fmodf(pos_x, 32), y*32 - fmodf(pos_y, 32),
          le_level->fg_tiles[ (y + (int)(pos_y / 32)) * le_level->get_sector("main")->solids->get_width() +       
          (x + (int)(pos_x / 32))],a);

      /* draw whats inside stuff when cursor is selecting those */
      /* (draw them all the time - is this the right behaviour?) */
      Tile* edit_image = TileManager::instance()->get(
          le_level->ia_tiles
          [ (y + (int)(pos_y / 32)) * le_level->get_section("main")->width + (x + (int)(pos_x / 32))]);
      if(edit_image && !edit_image->editor_images.empty())
        edit_image->editor_images[0]->draw( x * 32 - ((int)pos_x % 32), y*32 - ((int)pos_y % 32));

    }
#endif
  /* Draw the Bad guys: */
  for (std::vector<GameObject*>::iterator it = le_level->get_sector("main")->gameobjects.begin();
       it != le_level->get_sector("main")->gameobjects.end(); ++it)
  {
    BadGuy* badguy = dynamic_cast<BadGuy*> (*it);
    if(badguy == 0)
      continue;
    
    /* to support frames: img_bsod_left[(frame / 5) % 4] */
    badguy->draw(context);
  }

  /* Draw the player: */
  /* for now, the position is fixed at (100, 240) */
  largetux.walk_right->draw(context, Vector(100 - pos_x, 240 - pos_y), LAYER_OBJECTS-1);
}

void LevelEditor::change_object_properties(GameObject *pobj)
{
  DrawingContext context;
    
  Menu* object_properties_menu = new Menu();
  bool loop = true;

  std::string type = typeid(pobj).name();
  object_properties_menu->additem(MN_LABEL, type + " Properties",0,0);
  object_properties_menu->additem(MN_HL,"",0,0);

  BadGuy* pbad = dynamic_cast<BadGuy*>(pobj);
  if(pobj != 0)
  {
    object_properties_menu->additem(MN_STRINGSELECT,"Kind",0,0,1);
    for(int i = 0; i < NUM_BadGuyKinds; ++i)
    {
      string_list_add_item(object_properties_menu->get_item_by_id(1).list,
          badguykind_to_string(static_cast<BadGuyKind>(i)).c_str());
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

    //cap_screen->draw(0,0);

    object_properties_menu->draw(context);
    object_properties_menu->action();

    switch (object_properties_menu->check())
    {
    case 3:
      {
      BadGuy* pbad = dynamic_cast<BadGuy*>(pobj);
      if(pbad != 0) {
        BadGuy* pbad = dynamic_cast<BadGuy*>(pobj);
        pbad->kind =  badguykind_from_string(string_list_active(object_properties_menu->get_item_by_id(1).list));
        pbad->stay_on_platform = object_properties_menu->get_item_by_id(2).toggled;
      }
      loop = false;
      break;
      }
    default:
      break;
    }

    if(Menu::current() == NULL)
      loop = false;

    mouse_cursor->draw(context);
//    context.draw_filled_rect();
    SDL_Delay(25);
  }

  //delete cap_screen;
  Menu::set_current(0);
  delete object_properties_menu;
}


void LevelEditor::checkevents()
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
      if(!le_level && !Menu::current())
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
            Menu::set_current(leveleditor_menu);
            break;
          case SDLK_F1:
            if(le_level != NULL)
              showhelp();
            break;
          case SDLK_HOME:
            cursor_x = 0;
            pos_x = cursor_x;
            break;
          case SDLK_END:
            cursor_x = (le_level->get_sector("main")->solids->get_width() * 32) - 32;
            pos_x = cursor_x;
            break;
          case SDLK_F9:
            le_show_grid = !le_show_grid;
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
            selection.y1 = event.motion.y + pos_y;
            selection.x2 = event.motion.x + pos_x;
            selection.y2 = event.motion.y + pos_y;
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
              cursor_y = ((int)(pos_y + y) / 32) * 32;
            }
            else
            {
              cursor_x = x;
              cursor_y = y;
            }

            if(le_mouse_pressed[LEFT])
            {
              selection.x2 = x + pos_x;
              selection.y2 = y + pos_y;
            }

            if(le_mouse_pressed[RIGHT])
            {
              pos_x += -1 * event.motion.xrel;
              pos_y += -1 * event.motion.yrel;
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

    if(le_level != NULL)
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
            testlevel();
          le_save_level_bt->event(event);
          if(le_save_level_bt->get_state() == BUTTON_CLICKED)
            le_level->save(le_level_subset->name.c_str());
          le_exit_bt->event(event);
          if(le_exit_bt->get_state() == BUTTON_CLICKED)
          {
            Menu::set_current(leveleditor_menu);
          }
          le_next_level_bt->event(event);
          if(le_next_level_bt->get_state() == BUTTON_CLICKED)
          {
            if(le_levelnb < le_level_subset->get_num_levels())
            {
              goto_level(le_levelnb+1);
            }
            else
            {
              Level new_lev;
              char str[1024];
              sprintf(str,"Level %d doesn't exist. Create it?",le_levelnb+1);
              Surface* surf = new Surface(le_level->get_sector("main")->background->get_image(), false);
              if(confirm_dialog(surf, str))
              {
                le_level_subset->add_level("newlevel.stl");
                new_lev.save(le_level_subset->get_level_filename(le_levelnb+1));
                goto_level(le_levelnb);
              }
             if(surf != NULL)
              delete surf;
            }
          }
          le_previous_level_bt->event(event);
          if(le_previous_level_bt->get_state() == BUTTON_CLICKED)
          {
            if(le_levelnb > 1)
              goto_level(le_levelnb -1);
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
              change_object_properties(selected_game_object);
            }
          }


          if(le_selection_mode == SQUARE)
          {
            le_select_mode_one_bt->event(event);
            if(le_select_mode_one_bt->get_state() == BUTTON_CLICKED)
              le_selection_mode = CURSOR;
          }
          else
          {
            le_select_mode_two_bt->event(event);
            if(le_select_mode_two_bt->get_state() == BUTTON_CLICKED)
              le_selection_mode = SQUARE;
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
#if 0   // TODO FIXME XXX: New solution for this?
                le_current.Object(pbutton->get_drawable());
#endif
              }
            }
          }

          if((pbutton = le_tilemap_panel->event(event)) != NULL)
          {
            if(pbutton->get_state() == BUTTON_CLICKED)
            {
              active_tm = pbutton->get_tag();
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
              change(cursor_x, cursor_y, active_tm, le_current.tile);
          }
        }
        else if(le_mouse_clicked[LEFT])
        {
          if(MouseCursor::current() == mouse_select_object)
          {
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
            cursor_base.y = cursor_y + pos_y;	    
	    }
            cursor_base.width = 32;
            cursor_base.height = 32;

            for(std::vector<GameObject*>::iterator it =
                le_level->get_sector("main")->gameobjects.begin();
                it != le_level->get_sector("main")->gameobjects.end(); ++it) {
              MovingObject* mobj = dynamic_cast<MovingObject*> (*it);
              if(!mobj)
                continue;

              if(rectcollision(cursor_base, mobj->base))
              {
                selected_game_object = mobj;
                object_got_hit = true;
                break;
              }
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
            // FIXME TODO
#if 0
//FIXME: objects interactions with the level editor should have a major improvement
            if(le_current.IsObject())
            {
              le_level_changed  = true;
              BadGuy* pbadguy = dynamic_cast<BadGuy*>(le_current.obj);

              if(pbadguy)
              {
                Camera& camera = *le_level->get_sector("main")->camera;

                le_level->get_sector("main")->bad_guys.push_back(
                    new BadGuy(pbadguy->kind,
                      cursor_x + camera.get_translation().x,
                      cursor_y + camera.get_translation().y));
                le_level->get_sector("main")->gameobjects.push_back(le_level->get_sector("main")->bad_guys.back());
              }
            }
#endif

          }
	  
          le_mouse_clicked[LEFT] = false;

        }
      }
    }
  }
  if(!Menu::current())
  {
    show_minimap = false;

    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_TAB)
      show_minimap = true;

    le_move_left_bt->event(event);
    le_move_right_bt->event(event);
    le_move_up_bt->event(event);
    le_move_down_bt->event(event);
    switch(le_move_left_bt->get_state())
    {
    case BUTTON_PRESSED:
      pos_x -= KEYBOARD_SPEED;
      show_minimap = true;
      break;
    case BUTTON_HOVER:
      pos_x -= MOUSE_SPEED;
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
      pos_x += KEYBOARD_SPEED;
      show_minimap = true;
      break;
    case BUTTON_HOVER:
      pos_x += MOUSE_SPEED;
      show_minimap = true;
      break;
    case BUTTON_CLICKED:
      show_minimap = true;
      break;
    default:
      break;
    }

    switch(le_move_up_bt->get_state())
    {
    case BUTTON_PRESSED:
      pos_y -= KEYBOARD_SPEED;
      show_minimap = true;
      break;
    case BUTTON_HOVER:
      pos_y -= MOUSE_SPEED;
      show_minimap = true;
      break;
    case BUTTON_CLICKED:
      show_minimap = true;
      break;
    default:
      break;
    }

    switch(le_move_down_bt->get_state())
    {
    case BUTTON_PRESSED:
      pos_y += KEYBOARD_SPEED;
      show_minimap = true;
      break;
    case BUTTON_HOVER:
      pos_y += MOUSE_SPEED;
      show_minimap = true;
      break;
    case BUTTON_CLICKED:
      show_minimap = true;
      break;
    default:
      break;
    }

    /* checking if pos_x and pos_y is within the limits... */
    if((unsigned)pos_x > (le_level->get_sector("main")->solids->get_width() * 32 + 32*2) - screen->w)
      pos_x = (le_level->get_sector("main")->solids->get_width() * 32 + 32*2) - screen->w;
    if(pos_x < 0)
      pos_x = 0;

    if((unsigned)pos_y > (le_level->get_sector("main")->solids->get_height() * 32) - screen->h)
      pos_y = (le_level->get_sector("main")->solids->get_height() * 32) - screen->h;
    if(pos_y < 0)
      pos_y = 0;
  }
}

void LevelEditor::highlight_selection()
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

  fillrect(x1*32-pos_x, y1*32-pos_y,32* (x2 - x1 + 1),32 * (y2 - y1 + 1),173,234,177,103);
}

void LevelEditor::change(float x, float y, int tm, unsigned int c)
{
  if(le_level != NULL)
  {
    int xx,yy;
    int x1, x2, y1, y2;

    le_level_changed = true;

    switch(le_selection_mode)
    {
    case CURSOR:
      change(x,y,tm,c);

      base_type cursor_base;
      cursor_base.x = x;
      cursor_base.y = y;
      cursor_base.width = 32;
      cursor_base.height = 32;

      /* if there is a bad guy over there, remove it */
      // XXX TODO
      for(std::vector<GameObject*>::iterator it = le_level->get_sector("main")->gameobjects.begin();
            it != le_level->get_sector("main")->gameobjects.end(); ++it) {
        BadGuy* badguy = dynamic_cast<BadGuy*>((*it));
        if (badguy)
        {
          if(rectcollision(cursor_base, badguy->base))
          {
            delete (*it);
            le_level->get_sector("main")->gameobjects.erase(std::remove(le_level->get_sector("main")->gameobjects.begin(),
               le_level->get_sector("main")->gameobjects.end(), *it),
               le_level->get_sector("main")->gameobjects.end());
            break;
          }
        }
      }

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
      // TODO FIXME
      for(std::vector<GameObject*>::iterator it = le_level->get_sector("main")->gameobjects.begin();
          it != le_level->get_sector("main")->gameobjects.end(); ++it /* will be at end of loop */)
      {
        MovingObject* pmobject = dynamic_cast<MovingObject*> (*it);       
        if (pmobject)
        {
          if(pmobject->base.x/32 >= x1 && pmobject->base.x/32 <= x2
              && pmobject->base.y/32 >= y1 && pmobject->base.y/32 <= y2)
          {
            delete (*it);
            le_level->get_sector("main")->gameobjects.erase(std::remove(le_level->get_sector("main")->gameobjects.begin(), le_level->get_sector("main")->gameobjects.end(), *it), le_level->get_sector("main")->gameobjects.end());
            continue;
          }
          else
          {
            ++it;
          }
        }
      }

      for(xx = x1; xx <= x2; xx++)
        for(yy = y1; yy <= y2; yy++)
        {
          change(xx*32, yy*32, tm, c);

        }
      break;
    default:
      break;
    }
  }
}

void LevelEditor::testlevel()
{
  //Make sure a time value is set when testing the level
  if(le_level->time_left == 0)
    le_level->time_left = 250;

  le_level->save("test.stl");

  GameSession session("test.stl", ST_GL_TEST);
  session.run();
  player_status.reset();

  sound_manager->halt_music();

  Menu::set_current(NULL);
}

void LevelEditor::showhelp()
{
  DrawingContext context;

  bool tmp_show_grid = le_show_grid;
  SelectionMode temp_le_selection_mode = le_selection_mode;
  le_selection_mode = NONE;
  show_selections = true;
  le_show_grid = false;
  le_help_shown = true;

  SDL_Event event;
  unsigned int done_;
  char str[1024];
  char *text1[] = {

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

  char *text2[] = {

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

  char *text3[] = {

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

  char **text[] = { text1, text2, text3 };


  for(int i = 0; i < 3; i++)
    {
    context.draw_gradient(Color(0,0,0), Color(255,255,255), LAYER_BACKGROUND0);
    drawinterface(context);

    context.draw_text_center(blue_text, "- Help -", Vector(0, 30), LAYER_GUI);

    for(unsigned int t = 0; t < sizeof(text[i])/sizeof(char *); t++)
      context.draw_text(white_text, text[i][t], Vector(5, 80+(t*white_text->get_height())), LAYER_GUI);

    sprintf(str,"Press any key to continue - Page %d/%d?", i, static_cast<int>(sizeof(text)));
    context.draw_text(gold_text, str, Vector(0, 0), LAYER_GUI);

    context.do_drawing();

    done_ = 0;

    while(done_ == 0)
      {
      done_ = wait_for_event(event);
      SDL_Delay(50);
      }
    }

  show_selections = true;
  le_show_grid = tmp_show_grid;
  le_selection_mode = temp_le_selection_mode;
  le_help_shown = false;
}
