//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Bill Kendrick <bill@newbreedsoftware.com>
//                     Tobias Glaesser <tobi.web@gmx.de>
//                     Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef SUPERTUX_GLOBALS_H
#define SUPERTUX_GLOBALS_H

#include <string>
#include <SDL.h>
#include "text.h"
#include "menu.h"
#include "mousecursor.h"

#ifdef GP2X
#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_CLICK           (18)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (15)
#define GP2X_BUTTON_Y               (14)
#define GP2X_BUTTON_L               (10)
#define GP2X_BUTTON_R               (11)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)
#endif

extern std::string datadir;

struct JoystickKeymap
{
#ifndef GP2X
  int a_button;
  int b_button;
  int start_button;

  int x_axis;
  int y_axis;
  
  int dead_zone;

  JoystickKeymap();
#else
  int a_button;
  int b_button;
  int start_button;
  int up_button;
  int down_button;
  int left_button;
  int right_button;
  int volup_button;
  int voldown_button;

  JoystickKeymap();
#endif
};

extern JoystickKeymap joystick_keymap;

extern SDL_Surface * screen;
extern Text* black_text;
extern Text* gold_text;
extern Text* silver_text;
extern Text* white_text;
extern Text* white_small_text;
extern Text* white_big_text;
extern Text* blue_text;
extern Text* red_text;
extern Text* green_text;
extern Text* yellow_nums;

extern MouseCursor * mouse_cursor;

extern bool use_gl;
extern bool use_joystick;
extern bool use_fullscreen;
extern bool debug_mode;
extern bool show_fps;
extern bool show_mouse;

/** The number of the joystick that will be use in the game */
extern int joystick_num;
extern char* level_startup_file;
extern bool launch_leveleditor_mode;

/* SuperTux directory ($HOME/.supertux) and save directory($HOME/.supertux/save) */
extern char* st_dir;
extern char* st_save_dir;

extern float game_speed;
extern SDL_Joystick * js;

int wait_for_event(SDL_Event& event,unsigned int min_delay = 0, unsigned int max_delay = 0, bool empty_events = false);

#endif /* SUPERTUX_GLOBALS_H */
