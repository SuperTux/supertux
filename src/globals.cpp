//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 SuperTux Development Team, see AUTHORS for details
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

#include "globals.h"

/** The datadir prefix prepended when loading game data file */
std::string datadir;

JoystickKeymap::JoystickKeymap()
{
#ifndef GP2X
  a_button     = 0;
  b_button     = 1;
  start_button = 2;
  
  x_axis = 0;
  y_axis = 1;
    
  dead_zone = 4096;
#else
  a_button     = GP2X_BUTTON_A;
  b_button     = GP2X_BUTTON_X;
  start_button = GP2X_BUTTON_START;
  up_button    = GP2X_BUTTON_UP;
  down_button  = GP2X_BUTTON_DOWN;
  left_button  = GP2X_BUTTON_LEFT;
  right_button = GP2X_BUTTON_RIGHT;
  volup_button  = GP2X_BUTTON_VOLUP;
  voldown_button = GP2X_BUTTON_VOLDOWN;
#endif
}

JoystickKeymap joystick_keymap;

SDL_Surface * screen;
Text* black_text;
Text* gold_text;
Text* silver_text;
Text* blue_text;
Text* red_text;
Text* green_text;
Text* yellow_nums;
Text* white_text;
Text* white_small_text;
Text* white_big_text;

MouseCursor * mouse_cursor;

bool use_gl;
bool use_joystick;
bool use_fullscreen;
bool debug_mode;
bool show_fps;
bool show_mouse;
float game_speed = 1.0f;

int joystick_num = 0;
char* level_startup_file = 0;
bool launch_leveleditor_mode = false;

/* SuperTux directory ($HOME/.supertux-milestone1) and save directory($HOME/.supertux-milestone1/save) */
char *st_dir, *st_save_dir;

SDL_Joystick * js;

/* Returns 1 for every button event, 2 for a quit event and 0 for no event. */
int wait_for_event(SDL_Event& event,unsigned int min_delay, unsigned int max_delay, bool empty_events)
{
  int i;
  Timer maxdelay;
  Timer mindelay;
  
  maxdelay.init(false);
  mindelay.init(false);

  if(max_delay < min_delay)
    max_delay = min_delay;

  maxdelay.start(max_delay);
  mindelay.start(min_delay);

  if(empty_events)
    while (SDL_PollEvent(&event))
    {}

  /* Handle events: */

  for(i = 0; maxdelay.check() || !i; ++i)
    {
      while (SDL_PollEvent(&event))
        {
          if(!mindelay.check())
            {
              if (event.type == SDL_QUIT)
                {
                  /* Quit event - quit: */
                  return 2;
                }
              else if (event.type == SDL_KEYDOWN)
                {
                  /* Keypress - skip intro: */

                  return 1;
                }
              else if (event.type == SDL_JOYBUTTONDOWN)
                {
                  /* Fire button - skip intro: */

                  return 1;
                }
              else if (event.type == SDL_MOUSEBUTTONDOWN)
                {
                  /* Mouse button - skip intro: */
                  return 1;
                }
            }
        }
      SDL_Delay(10);
    }

  return 0;
}
