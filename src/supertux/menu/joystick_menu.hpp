//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>,
//                2007 Ingo Ruhnke <grumbel@gmail.com>
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

#include "control/input_manager.hpp"
#include "gui/menu_item.hpp"

class JoystickMenu final : public Menu
{
public:
  JoystickMenu(InputManager& input_manager, int player_id = 0);
  ~JoystickMenu() override;

  void refresh() override;
  void menu_action(MenuItem& item) override;

private:
  void refresh_control(const Control& control);
  std::string get_button_name(int button) const;

private:
  enum MenuIDs {
    MNID_JUMP_WITH_UP = static_cast<int>(Control::CONTROLCOUNT),
    MNID_SCAN_JOYSTICKS,
    MNID_AUTO_JOYSTICK_CFG
  };

private:
  InputManager& m_input_manager;
  int m_player_id;
  bool m_auto_joystick_cfg;

private:
  JoystickMenu(const JoystickMenu&) = delete;
  JoystickMenu& operator=(const JoystickMenu&) = delete;
};
