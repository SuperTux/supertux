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

#include "SDL.h"

#include "video/font.h"
#include "tinygettext.h"

namespace SuperTux
  {

  extern TinyGetText::DictionaryManager dictionary_manager;
  extern TinyGetText::Dictionary* dictionary;
  
  class MouseCursor;

  extern std::string datadir;

  struct JoystickKeymap
    {
      int a_button;
      int b_button;
      int start_button;

      int x_axis;
      int y_axis;

      int dead_zone;

      JoystickKeymap();
    };

  extern JoystickKeymap joystick_keymap;

  extern MouseCursor * mouse_cursor;

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

  /** The number of the joystick that will be use in the game */
  extern int joystick_num;

  /* SuperTux directory ($HOME/.supertux) */
  extern std::string user_dir;
  
  extern SDL_Joystick * js;

  int wait_for_event(SDL_Event& event,unsigned int min_delay = 0, unsigned int max_delay = 0, bool empty_events = false);

} //namespace SuperTux

#endif /* SUPERTUX_GLOBALS_H */
