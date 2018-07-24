//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2014 Ingo Ruhnke <grumbel@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SUPERTUX_CONTROL_JOYSTICK_MANAGER_HPP
#define HEADER_SUPERTUX_CONTROL_JOYSTICK_MANAGER_HPP

#include <SDL.h>
#include <vector>

#include "control/controller.hpp"

class InputManager;
class JoystickConfig;

class JoystickManager final
{
private:
  InputManager* parent;
  JoystickConfig& m_joystick_config;

  /// the number of buttons all joysticks have
  int min_joybuttons;

  /// the max number of buttons a joystick has
  int max_joybuttons;

  int max_joyaxis;
  int max_joyhats;

  Uint8 hat_state;

  int wait_for_joystick;

  std::vector<SDL_Joystick*> joysticks;

  friend class KeyboardManager;

public:
  JoystickManager(InputManager* parent, JoystickConfig& joystick_config);
  ~JoystickManager();

  void process_hat_event(const SDL_JoyHatEvent& jhat);
  void process_axis_event(const SDL_JoyAxisEvent& jaxis);
  void process_button_event(const SDL_JoyButtonEvent& jbutton);

  void bind_next_event_to(Controller::Control id);

  void set_joy_controls(Controller::Control id, bool value);

  void on_joystick_added(int joystick_index);
  void on_joystick_removed(int instance_id);

  int get_num_joysticks() const { return static_cast<int>(joysticks.size()); }

private:
  JoystickManager(const JoystickManager&) = delete;
  JoystickManager& operator=(const JoystickManager&) = delete;
};

#endif

/* EOF */
