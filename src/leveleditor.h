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

/* leveleditor.h - A built-in level editor for SuperTux */

#ifndef SUPERTUX_LEVELEDITOR_H
#define SUPERTUX_LEVELEDITOR_H

#include "video/drawing_context.h"
#include "special/game_object.h"
#include "video/surface.h"
#include "level.h"
#include "level_subset.h"
#include "special/moving_object.h"
#include "gui/button.h"
#include "gui/menu.h"

enum LevelEditorMainMenuIDs {
  MNID_RETURNLEVELEDITOR,
  MNID_SUBSETSETTINGS,
  MNID_QUITLEVELEDITOR
  };
  
enum LevelEditorSubsetSettingsIDs {
  MNID_SUBSETTITLE,
  MNID_SUBSETDESCRIPTION,
  MNID_SUBSETSAVECHANGES
  };
  
enum LevelEditorSubsetNewIDs {
 MNID_SUBSETNAME,
 MNID_CREATESUBSET
};

enum LevelEditorSettingsMenuIDs {
  MNID_NAME,
  MNID_AUTHOR,
  MNID_SONG,
  MNID_BGIMG,
  MNID_PARTICLE,
  MNID_LENGTH,
  MNID_HEIGHT,
  MNID_TIME,
  MNID_GRAVITY,
  MNID_BGSPEED,
  MNID_TopRed,
  MNID_TopGreen,
  MNID_TopBlue,
  MNID_BottomRed,
  MNID_BottomGreen,
  MNID_BottomBlue,
  MNID_APPLY
  };

  
class LevelEditor
{
public:
  LevelEditor();
  ~LevelEditor();

  int run(char* filename = NULL);

private:

// Functions
void newlevel(void);
void selectlevel(void);
void savelevel();
void editlevel(void);
void testlevel(void);
void checkevents(void);
void unload_level();

/* own declerations */
/* crutial ones (main loop) */
void init_menus();
int load_level_subset(char *filename);
void drawlevel(DrawingContext& context);
void drawinterface(DrawingContext& context);
void change(float x, float y, int tm, unsigned int c);
void showhelp();
void set_defaults(void);
void activate_bad_guys(void);
void goto_level(int levelnb);
void highlight_selection();

void drawminimap();

void apply_level_settings_menu();
void update_subset_settings_menu();
void save_subset_settings_menu();
void update_level_settings_menu();
void change_object_properties(GameObject *pobj);

// structs
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

struct square
{
  int x1, y1, x2, y2;
};

/* selection modes */
enum SelectionMode { CURSOR, SQUARE, NONE };

// variables
/* leveleditor internals */
string_list_type level_subsets;
bool le_level_changed;  /* if changes, ask for saving, when quiting*/
bool show_minimap;
bool show_selections;
bool le_help_shown;
int pos_x, pos_y, cursor_x, cursor_y;
int le_levelnb;
Level* le_level;
LevelSubset* le_level_subset;
int le_show_grid;
int le_frame;
Surface* le_selection;
int done;
TileOrObject le_current;
bool le_mouse_pressed[2];
bool le_mouse_clicked[2];
Button* le_save_level_bt;
Button* le_exit_bt;
Button* le_test_level_bt;
Button* le_next_level_bt;
Button* le_previous_level_bt;
Button* le_move_right_bt;
Button* le_move_left_bt;
Button* le_move_up_bt;
Button* le_move_down_bt;
Button* le_rubber_bt;
Button* le_select_mode_one_bt;
Button* le_select_mode_two_bt;
Button* le_settings_bt;
Button* le_tilegroup_bt;
Button* le_objects_bt;
Button* le_object_select_bt;
Button* le_object_properties_bt;
ButtonPanel* le_tilemap_panel;
int active_tm;
Menu* leveleditor_menu;
Menu* subset_load_menu;
Menu* subset_new_menu;
Menu* subset_settings_menu;
Menu* level_settings_menu;
Menu* select_tilegroup_menu;
Menu* select_objects_menu;
Timer select_tilegroup_menu_effect;
Timer select_objects_menu_effect;
Timer display_level_info;
typedef std::map<std::string, ButtonPanel*> ButtonPanelMap;
ButtonPanelMap tilegroups_map;
ButtonPanelMap objects_map;
std::string cur_tilegroup;
std::string cur_objects;
MouseCursor* mouse_select_object;
MovingObject* selected_game_object;

square selection;
SelectionMode le_selection_mode;
SDL_Event event;
};

#endif /*SUPERTUX_LEVELEDITOR_H*/
