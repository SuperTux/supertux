//  SuperTux
//  Copyright (C) 2013 Ingo Ruhnke <grumbel@gmx.de>
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

#include "control/joystick_manager.hpp"

#include <iostream>
#include <algorithm>

#include "control/joystickkeyboardcontroller.hpp"
#include "lisp/list_iterator.hpp"
#include "supertux/menu/joystick_menu.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "util/writer.hpp"

JoystickManager::JoystickManager(JoystickKeyboardController* parent) :
  parent(parent),
  joy_button_map(),
  joy_axis_map(),
  joy_hat_map(),
  dead_zone(4000),
  min_joybuttons(),
  max_joybuttons(),
  max_joyaxis(),
  max_joyhats(),
  hat_state(0),
  jump_with_up_joy(false),
  wait_for_joystick(-1),
  joysticks()
{
  // Default joystick button configuration
  bind_joybutton(0, 0, Controller::JUMP);
  bind_joybutton(0, 1, Controller::ACTION);
  // 6 or more Buttons
  if( min_joybuttons > 5 ){
    bind_joybutton(0, 4, Controller::PEEK_LEFT);
    bind_joybutton(0, 5, Controller::PEEK_RIGHT);
    // 8 or more
    if(min_joybuttons > 7)
      bind_joybutton(0, min_joybuttons-1, Controller::PAUSE_MENU);
  } else {
    // map the last 2 buttons to menu and pause
    if(min_joybuttons > 2)
      bind_joybutton(0, min_joybuttons-1, Controller::PAUSE_MENU);
    // map all remaining joystick buttons to MENU_SELECT
    for(int i = 2; i < max_joybuttons; ++i) {
      if(i != min_joybuttons-1)
        bind_joybutton(0, i, Controller::MENU_SELECT);
    }
  }

  // Default joystick axis configuration
  bind_joyaxis(0, -1, Controller::LEFT);
  bind_joyaxis(0, 1, Controller::RIGHT);
  bind_joyaxis(0, -2, Controller::UP);
  bind_joyaxis(0, 2, Controller::DOWN);
}

JoystickManager::~JoystickManager()
{
  for(auto joy : joysticks)
  {
    SDL_JoystickClose(joy);
  }
}

void
JoystickManager::on_joystick_added(int joystick_index)
{
  std::cout << "joydeviceadded: " << joystick_index << std::endl;
  SDL_Joystick* joystick = SDL_JoystickOpen(joystick_index);
  if (!joystick)
  {
    log_warning << "failed to open joystick: " << joystick_index
                << ": " << SDL_GetError() << std::endl;
  }
  else
  {
    joysticks.push_back(joystick);
  }

  if(min_joybuttons < 0 || SDL_JoystickNumButtons(joystick) < min_joybuttons)
    min_joybuttons = SDL_JoystickNumButtons(joystick);

  if(SDL_JoystickNumButtons(joystick) > max_joybuttons)
    max_joybuttons = SDL_JoystickNumButtons(joystick);

  if(SDL_JoystickNumAxes(joystick) > max_joyaxis)
    max_joyaxis = SDL_JoystickNumAxes(joystick);

  if(SDL_JoystickNumHats(joystick) > max_joyhats)
    max_joyhats = SDL_JoystickNumHats(joystick);
}

void
JoystickManager::on_joystick_removed(int instance_id)
{
  std::cout << "joydeviceremoved: " << static_cast<int>(instance_id) << std::endl;
  for(auto& joy : joysticks)
  {
    SDL_JoystickID id = SDL_JoystickInstanceID(joy);
    if (id == instance_id)
    {
      SDL_JoystickClose(joy);
      joy = nullptr;
    }
  }

  joysticks.erase(std::remove(joysticks.begin(), joysticks.end(), nullptr),
                  joysticks.end());
}

void
JoystickManager::process_hat_event(const SDL_JoyHatEvent& jhat)
{
  Uint8 changed = hat_state ^ jhat.value;

  if (wait_for_joystick >= 0)
  {
    if (changed & SDL_HAT_UP && jhat.value & SDL_HAT_UP)
      bind_joyhat(jhat.which, SDL_HAT_UP, Controller::Control(wait_for_joystick));

    if (changed & SDL_HAT_DOWN && jhat.value & SDL_HAT_DOWN)
      bind_joyhat(jhat.which, SDL_HAT_DOWN, Controller::Control(wait_for_joystick));

    if (changed & SDL_HAT_LEFT && jhat.value & SDL_HAT_LEFT)
      bind_joyhat(jhat.which, SDL_HAT_LEFT, Controller::Control(wait_for_joystick));

    if (changed & SDL_HAT_RIGHT && jhat.value & SDL_HAT_RIGHT)
      bind_joyhat(jhat.which, SDL_HAT_RIGHT, Controller::Control(wait_for_joystick));

    MenuStorage::get_joystick_options_menu()->update();
    wait_for_joystick = -1;
  }
  else
  {
    if (changed & SDL_HAT_UP)
    {
      HatMap::iterator it = joy_hat_map.find(std::make_pair(jhat.which, SDL_HAT_UP));
      if (it != joy_hat_map.end())
        set_joy_controls(it->second, jhat.value & SDL_HAT_UP);
    }

    if (changed & SDL_HAT_DOWN)
    {
      HatMap::iterator it = joy_hat_map.find(std::make_pair(jhat.which, SDL_HAT_DOWN));
      if (it != joy_hat_map.end())
        set_joy_controls(it->second, jhat.value & SDL_HAT_DOWN);
    }

    if (changed & SDL_HAT_LEFT)
    {
      HatMap::iterator it = joy_hat_map.find(std::make_pair(jhat.which, SDL_HAT_LEFT));
      if (it != joy_hat_map.end())
        set_joy_controls(it->second, jhat.value & SDL_HAT_LEFT);
    }

    if (changed & SDL_HAT_RIGHT)
    {
      HatMap::iterator it = joy_hat_map.find(std::make_pair(jhat.which, SDL_HAT_RIGHT));
      if (it != joy_hat_map.end())
        set_joy_controls(it->second, jhat.value & SDL_HAT_RIGHT);
    }
  }

  hat_state = jhat.value;
}

void
JoystickManager::process_axis_event(const SDL_JoyAxisEvent& jaxis)
{
  if (wait_for_joystick >= 0)
  {
    if (abs(jaxis.value) > dead_zone) {
      if (jaxis.value < 0)
        bind_joyaxis(jaxis.which, -(jaxis.axis + 1), Controller::Control(wait_for_joystick));
      else
        bind_joyaxis(jaxis.which, jaxis.axis + 1, Controller::Control(wait_for_joystick));

      MenuStorage::get_joystick_options_menu()->update();
      wait_for_joystick = -1;
    }
  }
  else
  {
    // Split the axis into left and right, so that both can be
    // mapped separately (needed for jump/down vs up/down)
    int axis = jaxis.axis + 1;

    AxisMap::iterator left  = joy_axis_map.find(std::make_pair(jaxis.which, -axis));
    AxisMap::iterator right = joy_axis_map.find(std::make_pair(jaxis.which, axis));

    if(left == joy_axis_map.end()) {
      // std::cout << "Unmapped joyaxis " << (int)jaxis.axis << " moved" << std::endl;
    } else {
      if (jaxis.value < -dead_zone)
        set_joy_controls(left->second,  true);
      else
        set_joy_controls(left->second, false);
    }

    if(right == joy_axis_map.end()) {
      // std::cout << "Unmapped joyaxis " << (int)jaxis.axis << " moved" << std::endl;
    } else {
      if (jaxis.value > dead_zone)
        set_joy_controls(right->second, true);
      else
        set_joy_controls(right->second, false);
    }
  }
}

void
JoystickManager::process_button_event(const SDL_JoyButtonEvent& jbutton)
{
  if(wait_for_joystick >= 0)
  {
    if(jbutton.state == SDL_PRESSED)
    {
      bind_joybutton(jbutton.which, jbutton.button, (Controller::Control)wait_for_joystick);
      MenuStorage::get_joystick_options_menu()->update();
      parent->reset();
      wait_for_joystick = -1;
    }
  }
  else
  {
    ButtonMap::iterator i = joy_button_map.find(std::make_pair(jbutton.which, jbutton.button));
    if(i == joy_button_map.end()) {
      log_debug << "Unmapped joybutton " << (int)jbutton.button << " pressed" << std::endl;
    } else {
      set_joy_controls(i->second, (jbutton.state == SDL_PRESSED));
    }
  }
}


int
JoystickManager::reversemap_joyaxis(Controller::Control c)
{
  for(AxisMap::iterator i = joy_axis_map.begin(); i != joy_axis_map.end(); ++i) {
    if(i->second == c)
      return i->first.second;
  }

  return 0;
}

int
JoystickManager::reversemap_joybutton(Controller::Control c)
{
  for(ButtonMap::iterator i = joy_button_map.begin(); i != joy_button_map.end(); ++i) {
    if(i->second == c)
      return i->first.second;
  }

  return -1;
}

int
JoystickManager::reversemap_joyhat(Controller::Control c)
{
  for(HatMap::iterator i = joy_hat_map.begin(); i != joy_hat_map.end(); ++i) {
    if(i->second == c)
      return i->first.second;
  }

  return -1;
}

void
JoystickManager::print_joystick_mappings()
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
JoystickManager::unbind_joystick_control(Controller::Control control)
{
  // remove all previous mappings for that control
  for(AxisMap::iterator i = joy_axis_map.begin(); i != joy_axis_map.end(); /* no ++i */) {
    if(i->second == control)
      joy_axis_map.erase(i++);
    else
      ++i;
  }

  for(ButtonMap::iterator i = joy_button_map.begin(); i != joy_button_map.end(); /* no ++i */) {
    if(i->second == control)
      joy_button_map.erase(i++);
    else
      ++i;
  }

  for(HatMap::iterator i = joy_hat_map.begin();  i != joy_hat_map.end(); /* no ++i */) {
    if(i->second == control)
      joy_hat_map.erase(i++);
    else
      ++i;
  }
}

void
JoystickManager::bind_joyaxis(JoyId joy_id, int axis, Controller::Control control)
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
JoystickManager::bind_joyhat(JoyId joy_id, int dir, Controller::Control c)
{
  unbind_joystick_control(c);

  // add new mapping
  joy_hat_map[std::make_pair(joy_id, dir)] = c;
}

void
JoystickManager::bind_joybutton(JoyId joy_id, int button, Controller::Control control)
{
  unbind_joystick_control(control);

  // add new mapping
  joy_button_map[std::make_pair(joy_id, button)] = control;
}

void
JoystickManager::read(const lisp::Lisp* joystick_lisp)
{
  joystick_lisp->get("dead-zone", dead_zone);
  joystick_lisp->get("jump-with-up", jump_with_up_joy);
  lisp::ListIterator iter(joystick_lisp);
  while(iter.next()) {
    if(iter.item() == _("map")) {
      int button = -1;
      int axis   = 0;
      int hat    = -1;
      std::string control;
      const lisp::Lisp* map = iter.lisp();

      map->get("control", control);
      int i = 0;
      for(i = 0; Controller::controlNames[i] != 0; ++i) {
        if(control == Controller::controlNames[i])
          break;
      }
      if(Controller::controlNames[i] == 0) {
        log_info << "Invalid control '" << control << "' in buttonmap" << std::endl;
        continue;
      }

      bool js_available = joysticks.size() > 0;

      if (map->get("button", button)) {
        if(js_available && (button < 0 || button >= max_joybuttons)) {
          log_info << "Invalid button '" << button << "' in buttonmap" << std::endl;
          continue;
        }
        bind_joybutton(0, button, Controller::Control(i));
      }

      if (map->get("axis",   axis)) {
        if (js_available && (axis == 0 || abs(axis) > max_joyaxis)) {
          log_info << "Invalid axis '" << axis << "' in axismap" << std::endl;
          continue;
        }
        bind_joyaxis(0, axis, Controller::Control(i));
      }

      if (map->get("hat",   hat)) {
        if (js_available        &&
            hat != SDL_HAT_UP   &&
            hat != SDL_HAT_DOWN &&
            hat != SDL_HAT_LEFT &&
            hat != SDL_HAT_RIGHT) {
          log_info << "Invalid axis '" << axis << "' in axismap" << std::endl;
          continue;
        } else {
          bind_joyhat(0, hat, Controller::Control(i));
        }
      }
    }
  }
}

void
JoystickManager::write(Writer& writer)
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

void
JoystickManager::set_joy_controls(Controller::Control id, bool value)
{
  if (jump_with_up_joy && id == Controller::UP)
  {
    parent->get_main_controller()->set_control(Controller::JUMP, value);
  }

  parent->get_main_controller()->set_control(id, value);
}

/* EOF */
