/***************************************************************************
                          leveleditor.h  -  built'in level editor
                             -------------------
    begin                : June, 23 2004
    copyright            : (C) 2004 by Ricardo Cruz
    email                : rick2@aeiou.pt
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SUPERTUX_LEVELEDITOR_H
#define SUPERTUX_LEVELEDITOR_H

#include "SDL.h"

#include <set>
#include <string>

#include "video/drawing_context.h"
#include "timer.h"
#include "level.h"
#include "level_subset.h"

using namespace SuperTux;

namespace SuperTux {
class ButtonGroup;
class Menu;
class Surface;
}

class Sector;
class TileMap;

enum {
  MN_ID_RETURN,
  MN_ID_LOAD_SUBSET,
  MN_ID_QUIT,

  // settings menu ids:
  MN_ID_NAME,
  MN_ID_AUTHOR,
  MN_ID_WIDTH,
  MN_ID_HEIGHT,
  MN_ID_APPLY_SETTINGS,
  
  // creating subset menu ids:
  MN_ID_FILENAME_SUBSET,
  MN_ID_TITLE_SUBSET,
  MN_ID_DESCRIPTION_SUBSET,
  MN_ID_CREATE_SUBSET
  };

enum {
  BT_LEVEL_SAVE,
  BT_LEVEL_TEST,
  BT_LEVEL_SETUP,

  BT_NEXT_LEVEL,
  BT_PREVIOUS_LEVEL,
  BT_NEXT_SECTOR,
  BT_PREVIOUS_SECTOR
  };

enum {
  OBJ_TRAMPOLINE = -100,
  OBJ_FLYING_PLATFORM = -101,
  OBJ_DOOR = -102
  };

class LevelEditor
{
public:
  LevelEditor();
  ~LevelEditor();

  void run(const std::string filename = "");

private:
  void events();
  void action();
  void draw(DrawingContext& context);

  void load_level_subset(std::string filename);
  void load_level(std::string filename);
  void load_level(int nb);
  void load_sector(std::string name);
  void load_sector(Sector* sector);

  void save_level();
  void test_level();
  void setup_level();

  void show_help();

  void change(int x, int y, int newtile, int layer);

  void load_buttons_gfx();
  void free_buttons_gfx();

  Level* level;
  std::string level_filename;

  Sector* sector;  // current sector
  TileMap *solids, *foregrounds, *backgrounds;
  std::string sector_name;

  std::set<std::string> level_subsets;
  LevelSubset* level_subset;
  int level_nb;

  Menu* main_menu;
  Menu* subset_menu;
  Menu* create_subset_menu;
  Menu* settings_menu;

  bool left_button, middle_button, mouse_moved;
  bool done;
  bool show_grid;

  Vector scroll;
  float zoom;

  SDL_Event event;
  Timer2 level_name_timer;

  Surface *img_background_bt, *img_foreground_bt, *img_interactive_bt;
  Surface *img_save_level_bt, *img_setup_level_bt, *img_test_level_bt;
  Surface *img_rubber_bt;
  Surface *img_previous_level_bt, *img_next_level_bt, *img_previous_sector_bt, *img_next_sector_bt;

  ButtonGroup *tiles_board, *tiles_layer, *level_options;
  int cur_layer;

  std::vector <std::vector <int> > selection;
  Vector selection_ini, selection_end;

  bool level_changed;
};

#endif
