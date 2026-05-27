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

#pragma once

#include <SDL.h>
#include <map>
#include <optional>
#include <unordered_map>

#include "control/controller.hpp"
#include "control/joystick_config.hpp"

class InputManager;


/**
 * Manages Joysticks.
 *
 * WARNING: Any edit done to this class should also be done to GameControllerManager!
 */
class JoystickManager final
{
  friend class KeyboardManager;

public:
  struct PlayerControl
  {
    int player_id;
    Control control;
  };

public:
  JoystickManager(InputManager* parent,
                  std::map<int, JoystickConfig>& joystick_configs);
  ~JoystickManager();

  void process_hat_event(const SDL_JoyHatEvent& jhat);
  void process_axis_event(const SDL_JoyAxisEvent& jaxis);
  void process_button_event(const SDL_JoyButtonEvent& jbutton);

  void bind_next_event_to(int player_id, Control id);

  void set_joy_controls(SDL_JoystickID joystick, Control id, bool value);

  void on_joystick_added(int joystick_index);
  void on_joystick_removed(int instance_id);

  inline int get_num_joysticks() const { return static_cast<int>(joysticks.size()); }

  void on_player_removed(int player_id);
  bool has_corresponding_joystick(int player_id) const;

  /** @returns 0 if success, 1 if controller doesn't support rumbling, 2 if game doesn't support rumbling */
  int rumble(SDL_Joystick* joystick) const;

  void bind_joystick(SDL_Joystick* joystick, int player_id);
  void rebind_joysticks();

  inline std::unordered_map<SDL_Joystick*, int>& get_joystick_mapping() { return joysticks; }

private:
  JoystickConfig& get_config_for_joystick(SDL_JoystickID instance_id);

  InputManager* parent;

  std::map<int, JoystickConfig>& m_joystick_configs;

  /// the number of buttons all joysticks have
  int min_joybuttons;

  /// the max number of buttons a joystick has
  int max_joybuttons;

  int max_joyaxis;
  int max_joyhats;

  Uint8 hat_state;

  std::optional<PlayerControl> wait_for_joystick;

  std::unordered_map<SDL_Joystick*, int> joysticks;

private:
  JoystickManager(const JoystickManager&) = delete;
  JoystickManager& operator=(const JoystickManager&) = delete;
};
