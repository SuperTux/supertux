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

#include <fmt/format.h>
#include <sstream>

#include "control/input_manager.hpp"
#include "control/joystick_manager.hpp"
#include "gui/item_controlfield.hpp"
#include "gui/item_toggle.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/gettext.hpp"

JoystickMenu::JoystickMenu(InputManager& input_manager, int player_id) :
  m_input_manager(input_manager),
  m_player_id(player_id),
  m_auto_joystick_cfg(!m_input_manager.use_game_controller())
{
  add_label(_("Setup Joystick"));
  add_hl();

  if (m_player_id == 0)
  {
    add_toggle(MNID_AUTO_JOYSTICK_CFG, _("Manual Configuration"),
             &m_auto_joystick_cfg)
      .set_help(_("Use manual configuration instead of SDL2's automatic GameController support"));
      add_hl();
  }

  if (m_input_manager.use_game_controller())
  {
    add_inactive(_("Enable Manual Configuration to setup joysticks"));
  }
  else
  {
    if (m_input_manager.joystick_manager->get_num_joysticks() > 0)
    {
      add_controlfield(static_cast<int>(Control::UP),          _("Up"));
      add_controlfield(static_cast<int>(Control::DOWN),        _("Down"));
      add_controlfield(static_cast<int>(Control::LEFT),        _("Left"));
      add_controlfield(static_cast<int>(Control::RIGHT),       _("Right"));
      add_controlfield(static_cast<int>(Control::JUMP),        _("Jump"));
      add_controlfield(static_cast<int>(Control::ACTION),      _("Action"));
      add_controlfield(static_cast<int>(Control::ITEM),        _("Item Pocket"));
      add_controlfield(static_cast<int>(Control::START),       _("Pause/Menu"));
      add_controlfield(static_cast<int>(Control::PEEK_LEFT),   _("Peek Left"));
      add_controlfield(static_cast<int>(Control::PEEK_RIGHT),  _("Peek Right"));
      add_controlfield(static_cast<int>(Control::PEEK_UP),     _("Peek Up"));
      add_controlfield(static_cast<int>(Control::PEEK_DOWN),   _("Peek Down"));

      if (m_player_id == 0 && g_config->developer_mode) {
        add_controlfield(static_cast<int>(Control::CONSOLE),    _("Console"));
        add_controlfield(static_cast<int>(Control::CHEAT_MENU), _("Cheat Menu"));
        add_controlfield(static_cast<int>(Control::DEBUG_MENU), _("Debug Menu"));
      }

      add_toggle(MNID_JUMP_WITH_UP, _("Jump with Up"), &g_config->joystick_configs[m_player_id].m_jump_with_up_joy);
    }
    else
    {
      add_inactive(_("No Joysticks found"));
    }
  }

  if (m_player_id == 0)
  {
    if (m_input_manager.get_num_users() > 1)
    {
     add_hl();
    }

    for (int id = 1; id < m_input_manager.get_num_users(); id++)
    {
      add_entry(fmt::format(fmt::runtime(_("Player {}")), std::to_string(id + 1)),
      [&input_manager, id] {
        MenuManager::instance().push_menu(
          std::make_unique<JoystickMenu>(input_manager, id));
      });
    }

    add_hl();
    add_toggle(-1, _("Globally Ignore Joystick Axis"), &g_config->ignore_joystick_axis);
    add_hl();

    add_entry(MNID_SCAN_JOYSTICKS, _("Scan for Joysticks"));
  }
  
  add_hl();
  add_back(_("Back"));
  refresh();
}

JoystickMenu::~JoystickMenu()
{}

std::string
JoystickMenu::get_button_name(int button) const
{
  if (button < 0)
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
JoystickMenu::menu_action(MenuItem& item)
{
  if (0 <= item.get_id() && item.get_id() < static_cast<int>(Control::CONTROLCOUNT))
  {
    ItemControlField& field = static_cast<ItemControlField&>(item);
    field.change_input(_("Press Button"));
    m_input_manager.joystick_manager->bind_next_event_to(m_player_id, static_cast<Control>(item.get_id()));
  }
  else if (item.get_id() == MNID_AUTO_JOYSTICK_CFG)
  {
    m_input_manager.use_game_controller(!m_auto_joystick_cfg);
    m_input_manager.reset();
    MenuManager::instance().set_menu(std::make_unique<JoystickMenu>(m_input_manager, m_player_id));
  }
  else if (item.get_id() == MNID_SCAN_JOYSTICKS)
  {
    m_input_manager.reset();
    MenuManager::instance().set_menu(std::make_unique<JoystickMenu>(m_input_manager, m_player_id));
  }
}

void
JoystickMenu::refresh_control(const Control& control)
{
  ItemControlField* itemcf = dynamic_cast<ItemControlField*>(&get_item_by_id(static_cast<int>(control)));
  if (!itemcf) {
    return;
  }

  int button  = g_config->joystick_configs[m_player_id].reversemap_joybutton(control);
  int axis    = g_config->joystick_configs[m_player_id].reversemap_joyaxis(control);
  int hat_dir = g_config->joystick_configs[m_player_id].reversemap_joyhat(control);

  if (button != -1)
  {
    itemcf->change_input(get_button_name(button));
  }
  else if (axis != 0)
  {
    std::ostringstream name;

    name << _("Axis ");

    if (axis < 0)
      name << _("-");
    else
      name << _("+");

    if (abs(axis) == 1)
      name << _("X");
    else if (abs(axis) == 2)
      name << _("Y");
    else if (abs(axis) == 3)
      name << _("X2");
    else if (abs(axis) == 4)
      name << _("Y2");
    else
      name << abs(axis);

    itemcf->change_input(name.str());
  }
  else if (hat_dir != -1)
  {
    std::string name;

    switch (hat_dir)
    {
      case SDL_HAT_UP:
        name = _("Hat Up");
        break;

      case SDL_HAT_DOWN:
        name = _("Hat Down");
        break;

      case SDL_HAT_LEFT:
        name = _("Hat Left");
        break;

      case SDL_HAT_RIGHT:
        name = _("Hat Right");
        break;

      default:
        name = "Unknown hat_dir";
        break;
    }

    itemcf->change_input(name);
  }
  else
  {
    itemcf->change_input(_("None"));
  }
}

void
JoystickMenu::refresh()
{
  if (m_input_manager.use_game_controller() ||
      m_input_manager.joystick_manager->get_num_joysticks() == 0)
  {
    return;
  }

  const auto& controls = { Control::UP, Control::DOWN, Control::LEFT, Control::RIGHT,
                           Control::JUMP, Control::ACTION, Control::ITEM,
                           Control::START, Control::PEEK_LEFT, Control::PEEK_RIGHT,
                           Control::PEEK_UP, Control::PEEK_DOWN };

  const auto& developer_controls = { Control::CHEAT_MENU, Control::DEBUG_MENU, Control::CONSOLE };

  for (const auto& control : controls)
  {
    refresh_control(control);
  }

  if (g_config->developer_mode && m_player_id == 0)
  {
    for (const auto& control : developer_controls)
    {
      refresh_control(control);
    }
  }
}
