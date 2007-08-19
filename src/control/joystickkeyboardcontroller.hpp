//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef __JOYSTICKKEYBOARDCONTROLLER_H__
#define __JOYSTICKKEYBOARDCONTROLLER_H__

#include "controller.hpp"
#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"
#include <SDL.h>
#include <string>
#include <map>

class Menu;

class JoystickKeyboardController : public Controller
{
public:
  JoystickKeyboardController();
  virtual ~JoystickKeyboardController();

  /** Process an SDL Event and return true if the event has been used
   */
  void process_event(const SDL_Event& event);

  void write(lisp::Writer& writer);
  void read(const lisp::Lisp& lisp);
  void reset();

  Menu* get_key_options_menu();
  Menu* get_joystick_options_menu();

private:
  void process_key_event(const SDL_Event& event);
  void process_console_key_event(const SDL_Event& event);
  void process_menu_key_event(const SDL_Event& event);

  typedef std::map<SDLKey, Control> KeyMap;
  KeyMap keymap;

  typedef std::map<int, Control> ButtonMap;
  ButtonMap joy_button_map;

  typedef std::map<int, Control> AxisMap;
  AxisMap joy_axis_map;

  std::vector<SDL_Joystick*> joysticks;

  std::string name;
  bool use_hat;
  int dead_zone;
  /// the number of buttons all joysticks have
  int min_joybuttons;
  /// the max number of buttons a joystick has
  int max_joybuttons;

  int max_joyaxis;

  SDLKey reversemap_key(Control c);
  int    reversemap_joybutton(Control c);
  int    reversemap_joyaxis(Control c);

  void reset_joybutton(int button, Control c);
  void reset_joyaxis(int axis, Control c);
  void reset_key(SDLKey key, Control c);

  int wait_for_key;
  int wait_for_joystick;

  class KeyboardMenu;
  class JoystickMenu;

  KeyboardMenu* key_options_menu;
  JoystickMenu* joystick_options_menu;
  friend class KeyboardMenu;
  friend class JoystickMenu;
};

#endif
