//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SUPERTUX_CONTROL_GAME_CONTROLLER_MANAGER_HPP
#define HEADER_SUPERTUX_CONTROL_GAME_CONTROLLER_MANAGER_HPP

#include <vector>

#include "SDL.h"

class InputManager;

class GameControllerManager
{
private:
  InputManager* m_parent;
  int m_deadzone;
  std::vector<SDL_GameController*> m_game_controllers;

public:
  GameControllerManager(InputManager* parent);
  ~GameControllerManager();

  void process_button_event(const SDL_ControllerButtonEvent& ev);
  void process_axis_event(const SDL_ControllerAxisEvent& ev);

  void on_controller_added(int joystick_index);
  void on_controller_removed(int instance_id);

private:
  GameControllerManager(const GameControllerManager&) = delete;
  GameControllerManager& operator=(const GameControllerManager&) = delete;
};

#endif

/* EOF */
