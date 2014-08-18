//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>,
//           2007,2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/menu/joystick_menu.hpp"

#include <sstream>

#include "control/joystick_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "util/gettext.hpp"

namespace {

enum {
  MNID_JUMP_WITH_UP = Controller::CONTROLCOUNT,
  MNID_SCAN_JOYSTICKS,
  MNID_AUTO_JOYSTICK_CFG
};

} // namespace

JoystickMenu::JoystickMenu(InputManager& input_manager) :
  m_input_manager(input_manager),
  m_joysticks_available(false)
{
  recreate_menu();
}

JoystickMenu::~JoystickMenu()
{}

void
JoystickMenu::recreate_menu()
{
  clear();
  add_label(_("Setup Joystick"));
  add_hl();

  add_toggle(MNID_AUTO_JOYSTICK_CFG, _("Manual Configuration"),
             !m_input_manager.use_game_controller())
    ->set_help(_("Use manual configuration instead of SDL2's automatic GameController support"));

  if (m_input_manager.use_game_controller())
  {
    m_joysticks_available = false;
  }
  else
  {
    if (m_input_manager.joystick_manager->joysticks.size() > 0)
    {
      m_joysticks_available = true;

      add_controlfield(Controller::UP,          _("Up"));
      add_controlfield(Controller::DOWN,        _("Down"));
      add_controlfield(Controller::LEFT,        _("Left"));
      add_controlfield(Controller::RIGHT,       _("Right"));
      add_controlfield(Controller::JUMP,        _("Jump"));
      add_controlfield(Controller::ACTION,      _("Action"));
      add_controlfield(Controller::PAUSE_MENU,  _("Pause/Menu"));
      add_controlfield(Controller::PEEK_LEFT,   _("Peek Left"));
      add_controlfield(Controller::PEEK_RIGHT,  _("Peek Right"));
      add_controlfield(Controller::PEEK_UP,     _("Peek Up"));
      add_controlfield(Controller::PEEK_DOWN,   _("Peek Down"));

      add_toggle(MNID_JUMP_WITH_UP, _("Jump with Up"), g_config->joystick_config.jump_with_up_joy);
    }
    else
    {
      m_joysticks_available = false;

      add_inactive(-1, _("No Joysticks found"));
      add_entry(MNID_SCAN_JOYSTICKS, _("Scan for Joysticks"));
    }
  }

  add_hl();
  add_back(_("Back"));
  refresh();
}

std::string
JoystickMenu::get_button_name(int button)
{
  if(button < 0)
  {
    return _("None");
  }
  else
  {
    std::ostringstream name;
    name << "Button " << button;
    return name.str();
  }
}

void
JoystickMenu::menu_action(MenuItem* item)
{
  if (0 <= item->id && item->id < Controller::CONTROLCOUNT)
  {
    item->change_input(_("Press Button"));
    m_input_manager.joystick_manager->wait_for_joystick = item->id;
  }
  else if (item->id == MNID_JUMP_WITH_UP)
  {
    g_config->joystick_config.jump_with_up_joy = item->toggled;
  }
  else if (item->id == MNID_AUTO_JOYSTICK_CFG)
  {
    m_input_manager.use_game_controller(!item->toggled);
    m_input_manager.reset();
    recreate_menu();
  }
  else if(item->id == MNID_SCAN_JOYSTICKS)
  {
    m_input_manager.reset();
    recreate_menu();
  }
}

void
JoystickMenu::refresh_menu_item(Controller::Control id)
{
  int button  = g_config->joystick_config.reversemap_joybutton(id);
  int axis    = g_config->joystick_config.reversemap_joyaxis(id);
  int hat_dir = g_config->joystick_config.reversemap_joyhat(id);

  if (button != -1)
  {
    get_item_by_id(static_cast<int>(id)).change_input(get_button_name(button));
  }
  else if (axis != 0)
  {
    std::ostringstream name;

    name << "Axis ";

    if (axis < 0)
      name << "-";
    else
      name << "+";

    if (abs(axis) == 1)
      name << "X";
    else if (abs(axis) == 2)
      name << "Y";
    else if (abs(axis) == 2)
      name << "X2";
    else if (abs(axis) == 3)
      name << "Y2";
    else
      name << abs(axis);

    get_item_by_id(static_cast<int>(id)).change_input(name.str());
  }
  else if (hat_dir != -1)
  {
    std::string name;

    switch (hat_dir)
    {
      case SDL_HAT_UP:
        name = "Hat Up";
        break;

      case SDL_HAT_DOWN:
        name = "Hat Down";
        break;

      case SDL_HAT_LEFT:
        name = "Hat Left";
        break;

      case SDL_HAT_RIGHT:
        name = "Hat Right";
        break;

      default:
        name = "Unknown hat_dir";
        break;
    }

    get_item_by_id(static_cast<int>(id)).change_input(name);
  }
  else
  {
    get_item_by_id(static_cast<int>(id)).change_input("None");
  }
}

void
JoystickMenu::refresh()
{
  if (m_joysticks_available)
  {
    refresh_menu_item(Controller::UP);
    refresh_menu_item(Controller::DOWN);
    refresh_menu_item(Controller::LEFT);
    refresh_menu_item(Controller::RIGHT);

    refresh_menu_item(Controller::JUMP);
    refresh_menu_item(Controller::ACTION);
    refresh_menu_item(Controller::PAUSE_MENU);
    refresh_menu_item(Controller::PEEK_LEFT);
    refresh_menu_item(Controller::PEEK_RIGHT);
    refresh_menu_item(Controller::PEEK_UP);
    refresh_menu_item(Controller::PEEK_DOWN);
  }
}

/* EOF */
