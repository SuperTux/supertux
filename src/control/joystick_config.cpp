//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "control/joystick_config.hpp"

#include <iostream>

#include "lisp/list_iterator.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

JoystickConfig::JoystickConfig() :
  dead_zone(8000),
  jump_with_up_joy(false),
  joy_button_map(),
  joy_axis_map(),
  joy_hat_map()
{
  // Default joystick button configuration
  bind_joybutton(0, 0, Controller::JUMP);
  bind_joybutton(0, 0, Controller::MENU_SELECT);
  bind_joybutton(0, 1, Controller::ACTION);
  bind_joybutton(0, 4, Controller::PEEK_LEFT);
  bind_joybutton(0, 5, Controller::PEEK_RIGHT);
  bind_joybutton(0, 6, Controller::PAUSE_MENU);

  // Default joystick axis configuration
  bind_joyaxis(0, -1, Controller::LEFT);
  bind_joyaxis(0, 1, Controller::RIGHT);
  bind_joyaxis(0, -2, Controller::UP);
  bind_joyaxis(0, 2, Controller::DOWN);
}

int
JoystickConfig::reversemap_joyaxis(Controller::Control c)
{
  for(AxisMap::iterator i = joy_axis_map.begin(); i != joy_axis_map.end(); ++i) {
    if (i->second == c)
      return i->first.second;
  }

  return 0;
}

int
JoystickConfig::reversemap_joybutton(Controller::Control c)
{
  for(ButtonMap::iterator i = joy_button_map.begin(); i != joy_button_map.end(); ++i) {
    if (i->second == c)
      return i->first.second;
  }

  return -1;
}

int
JoystickConfig::reversemap_joyhat(Controller::Control c)
{
  for(HatMap::iterator i = joy_hat_map.begin(); i != joy_hat_map.end(); ++i) {
    if (i->second == c)
      return i->first.second;
  }

  return -1;
}

void
JoystickConfig::print_joystick_mappings()
{
  std::cout << _("Joystick Mappings") << std::endl;
  std::cout << "-----------------" << std::endl;
  for(AxisMap::iterator i = joy_axis_map.begin(); i != joy_axis_map.end(); ++i) {
    std::cout << "Axis: " << i->first.second << " -> " << i->second << std::endl;
  }

  for(ButtonMap::iterator i = joy_button_map.begin(); i != joy_button_map.end(); ++i) {
    std::cout << "Button: " << i->first.second << " -> " << i->second << std::endl;
  }

  for(HatMap::iterator i = joy_hat_map.begin(); i != joy_hat_map.end(); ++i) {
    std::cout << "Hat: " << i->first.second << " -> " << i->second << std::endl;
  }
  std::cout << std::endl;
}

void
JoystickConfig::unbind_joystick_control(Controller::Control control)
{
  // remove all previous mappings for that control
  for(AxisMap::iterator i = joy_axis_map.begin(); i != joy_axis_map.end(); /* no ++i */) {
    if (i->second == control)
      joy_axis_map.erase(i++);
    else
      ++i;
  }

  for(ButtonMap::iterator i = joy_button_map.begin(); i != joy_button_map.end(); /* no ++i */) {
    if (i->second == control)
      joy_button_map.erase(i++);
    else
      ++i;
  }

  for(HatMap::iterator i = joy_hat_map.begin();  i != joy_hat_map.end(); /* no ++i */) {
    if (i->second == control)
      joy_hat_map.erase(i++);
    else
      ++i;
  }
}

void
JoystickConfig::bind_joyaxis(JoyId joy_id, int axis, Controller::Control control)
{
  // axis isn't the SDL axis number, but axisnumber + 1 with sign
  // changed depending on if the positive or negative end is to be
  // used (negative axis 0 becomes -1, positive axis 2 becomes +3,
  // etc.)

  unbind_joystick_control(control);

  // add new mapping
  joy_axis_map[std::make_pair(joy_id, axis)] = control;
}

void
JoystickConfig::bind_joyhat(JoyId joy_id, int dir, Controller::Control c)
{
  unbind_joystick_control(c);

  // add new mapping
  joy_hat_map[std::make_pair(joy_id, dir)] = c;
}

void
JoystickConfig::bind_joybutton(JoyId joy_id, int button, Controller::Control control)
{
  unbind_joystick_control(control);

  // add new mapping
  joy_button_map[std::make_pair(joy_id, button)] = control;
}

void
JoystickConfig::read(const lisp::Lisp& joystick_lisp)
{
  joystick_lisp.get("dead-zone", dead_zone);
  joystick_lisp.get("jump-with-up", jump_with_up_joy);

  lisp::ListIterator iter(&joystick_lisp);
  while(iter.next())
  {
    if (iter.item() == "map")
    {
      int button = -1;
      int axis   = 0;
      int hat    = -1;
      std::string control;
      const lisp::Lisp* map = iter.lisp();

      map->get("control", control);
      int i = 0;
      for(i = 0; Controller::controlNames[i] != 0; ++i)
      {
        if (control == Controller::controlNames[i])
          break;
      }

      if (Controller::controlNames[i] == 0)
      {
        log_info << "Invalid control '" << control << "' in buttonmap" << std::endl;
      }
      else
      {
        if (map->get("button", button))
        {
          bind_joybutton(0, button, Controller::Control(i));
        }
        else if (map->get("axis",   axis))
        {
          bind_joyaxis(0, axis, Controller::Control(i));
        }
        else if (map->get("hat",   hat))
        {
          if (hat != SDL_HAT_UP   &&
              hat != SDL_HAT_DOWN &&
              hat != SDL_HAT_LEFT &&
              hat != SDL_HAT_RIGHT) {
            log_info << "Invalid axis '" << axis << "' in axismap" << std::endl;
          }
          else
          {
            bind_joyhat(0, hat, Controller::Control(i));
          }
        }
      }
    }
  }
}

void
JoystickConfig::write(Writer& writer)
{
  writer.write("dead-zone", dead_zone);
  writer.write("jump-with-up", jump_with_up_joy);

  for(ButtonMap::iterator i = joy_button_map.begin(); i != joy_button_map.end();
      ++i) {
    writer.start_list("map");
    writer.write("button", i->first.second);
    writer.write("control", Controller::controlNames[i->second]);
    writer.end_list("map");
  }

  for(HatMap::iterator i = joy_hat_map.begin(); i != joy_hat_map.end(); ++i) {
    writer.start_list("map");
    writer.write("hat", i->first.second);
    writer.write("control", Controller::controlNames[i->second]);
    writer.end_list("map");
  }

  for(AxisMap::iterator i = joy_axis_map.begin(); i != joy_axis_map.end(); ++i) {
    writer.start_list("map");
    writer.write("axis", i->first.second);
    writer.write("control", Controller::controlNames[i->second]);
    writer.end_list("map");
  }
}

/* EOF */
