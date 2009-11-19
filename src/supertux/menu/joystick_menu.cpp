//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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

#include "util/gettext.hpp"
#include "supertux/gameconfig.hpp"

namespace{
  const int SCAN_JOYSTICKS = Controller::CONTROLCOUNT + 1;
}

JoystickMenu::JoystickMenu(JoystickKeyboardController* _controller) :
  controller(_controller)
{
  recreateMenu();
}

JoystickMenu::~JoystickMenu()
{}

void
JoystickMenu::recreateMenu()
{
  clear();
  add_label(_("Setup Joystick"));
  add_hl();
  if(controller->joysticks.size() > 0) {
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

    add_toggle(Controller::CONTROLCOUNT, _("Jump with Up"), controller->jump_with_up_joy);
  } else {
    add_inactive(-1, _("No Joysticks found"));
  }
  add_inactive(-1,"");
  add_entry(SCAN_JOYSTICKS, _("Scan for Joysticks"));

  //Show Joysticks currently activated:
  for(std::vector<SDL_Joystick*>::iterator i = controller->joysticks.begin();
      i != controller->joysticks.end(); ++i) {
    if(*i != 0)
      add_inactive(-1, SDL_JoystickName(SDL_JoystickIndex(*i)) );
  }

  add_hl();
  add_back(_("Back"));
  update();
}

std::string
JoystickMenu::get_button_name(int button)
{
  if(button < 0)
    return _("None");

  std::ostringstream name;
  name << "Button " << button;
  return name.str();
}

void
JoystickMenu::menu_action(MenuItem* item)
{
  if (item->id >= 0 && item->id < Controller::CONTROLCOUNT) {
    item->change_input(_("Press Button"));
    controller->wait_for_joystick = item->id;
  } else if (item->id == Controller::CONTROLCOUNT) {
    controller->jump_with_up_joy = item->toggled;
  } else if( item->id == SCAN_JOYSTICKS) {
    controller->updateAvailableJoysticks();
    recreateMenu();
  }
}

void
JoystickMenu::update_menu_item(Controller::Control id)
{
  int button  = controller->reversemap_joybutton(id);
  int axis    = controller->reversemap_joyaxis(id);
  int hat_dir = controller->reversemap_joyhat(id);

  if (button != -1) {
    get_item_by_id((int)id).change_input(get_button_name(button));
  } else if (axis != 0) {
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

    get_item_by_id((int)id).change_input(name.str());
  } else if (hat_dir != -1) {
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

    get_item_by_id((int)id).change_input(name);
  } else {
    get_item_by_id((int)id).change_input("None");
  }
}

void
JoystickMenu::update()
{
  if(controller->joysticks.size() == 0)
    return;

  update_menu_item(Controller::UP);
  update_menu_item(Controller::DOWN);
  update_menu_item(Controller::LEFT);
  update_menu_item(Controller::RIGHT);

  update_menu_item(Controller::JUMP);
  update_menu_item(Controller::ACTION);
  update_menu_item(Controller::PAUSE_MENU);
  update_menu_item(Controller::PEEK_LEFT);
  update_menu_item(Controller::PEEK_RIGHT);
  update_menu_item(Controller::PEEK_UP);
  update_menu_item(Controller::PEEK_DOWN);

  get_item_by_id(Controller::CONTROLCOUNT).toggled = controller->jump_with_up_joy;
}

/* EOF */
