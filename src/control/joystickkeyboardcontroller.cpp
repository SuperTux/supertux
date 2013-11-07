//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>,
//                2007 Ingo Ruhnke <grumbel@gmx.de>
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

#include "control/joystickkeyboardcontroller.hpp"

#include <iostream>

#include "lisp/list_iterator.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/console.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/joystick_menu.hpp"
#include "supertux/menu/keyboard_menu.hpp"
#include "util/gettext.hpp"
#include "util/writer.hpp"
//#include <SDL_keycode.h> // add by giby

JoystickKeyboardController::JoystickKeyboardController() :
  controller(),
  keymap(),
  joy_button_map(),
  joy_axis_map(),
  joy_hat_map(),
  joysticks(),
  name(),
  dead_zone(),
  min_joybuttons(),
  max_joybuttons(),
  max_joyaxis(),
  max_joyhats(),
  hat_state(0),
  jump_with_up_joy(),
  jump_with_up_kbd(),
  wait_for_key(-1), 
  wait_for_joystick(-1)
{
  controller = new Controller;

  // initialize default keyboard map
  keymap[SDLK_LEFT]     = Controller::LEFT;
  keymap[SDLK_RIGHT]    = Controller::RIGHT;
  keymap[SDLK_UP]       = Controller::UP;
  keymap[SDLK_DOWN]     = Controller::DOWN;
  keymap[SDLK_SPACE]    = Controller::JUMP;
  keymap[SDLK_LCTRL]    = Controller::ACTION;
  keymap[SDLK_LALT]     = Controller::ACTION;
  keymap[SDLK_ESCAPE]   = Controller::PAUSE_MENU;
  keymap[SDLK_p]        = Controller::PAUSE_MENU;
  keymap[SDLK_PAUSE]    = Controller::PAUSE_MENU;
  keymap[SDLK_RETURN]   = Controller::MENU_SELECT;
  keymap[SDLK_KP_ENTER] = Controller::MENU_SELECT;
  keymap[SDLK_CARET]    = Controller::CONSOLE;
  keymap[SDLK_DELETE]   = Controller::PEEK_LEFT;
  keymap[SDLK_PAGEDOWN] = Controller::PEEK_RIGHT;
  keymap[SDLK_HOME]     = Controller::PEEK_UP;
  keymap[SDLK_END]      = Controller::PEEK_DOWN;

  jump_with_up_joy = false;
  jump_with_up_kbd = false;

  updateAvailableJoysticks();

  dead_zone = 1000;

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

JoystickKeyboardController::~JoystickKeyboardController()
{
  for(std::vector<SDL_Joystick*>::iterator i = joysticks.begin();
      i != joysticks.end(); ++i) {
    if(*i != 0)
      SDL_JoystickClose(*i);
  }
  delete controller;
}

void
JoystickKeyboardController::updateAvailableJoysticks()
{
  for(std::vector<SDL_Joystick*>::iterator i = joysticks.begin();
      i != joysticks.end(); ++i) {
    if(*i != 0)
      SDL_JoystickClose(*i);
  }
  joysticks.clear();
  
  SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);

  int joystick_count = SDL_NumJoysticks();
  min_joybuttons = -1;
  max_joybuttons = -1;
  max_joyaxis    = -1;
  max_joyhats    = -1;

  if( joystick_count > 0 ){
    for(int i = 0; i < joystick_count; ++i) {
      SDL_Joystick* joystick = SDL_JoystickOpen(i);
      bool good = true;
      if(SDL_JoystickNumButtons(joystick) < 2) {
        log_info << _("Joystick ") << i << ": " << SDL_JoystickID(i) << _(" has less than 2 buttons") << std::endl;
        good = false;
      }
      if(SDL_JoystickNumAxes(joystick) < 2
         && SDL_JoystickNumHats(joystick) == 0) {
        log_info << _("Joystick ") << i << ": " << SDL_JoystickID(i) << _(" has less than 2 axes and no hat") << std::endl;
        good = false;
      }
      if(!good) {
        SDL_JoystickClose(joystick);
        continue;
      }

      if(min_joybuttons < 0 || SDL_JoystickNumButtons(joystick) < min_joybuttons)
        min_joybuttons = SDL_JoystickNumButtons(joystick);

      if(SDL_JoystickNumButtons(joystick) > max_joybuttons)
        max_joybuttons = SDL_JoystickNumButtons(joystick);

      if(SDL_JoystickNumAxes(joystick) > max_joyaxis)
        max_joyaxis = SDL_JoystickNumAxes(joystick);

      if(SDL_JoystickNumHats(joystick) > max_joyhats)
        max_joyhats = SDL_JoystickNumHats(joystick);

      joysticks.push_back(joystick);
    }
  }

  // some joysticks or SDL seem to produce some bogus events after being opened
  Uint32 ticks = SDL_GetTicks();
  while(SDL_GetTicks() - ticks < 200) {
    SDL_Event event;
    SDL_PollEvent(&event);
  }
}

Controller*
JoystickKeyboardController::get_main_controller()
{
  return controller;
}

void
JoystickKeyboardController::read(const Reader& lisp)
{
  const lisp::Lisp* keymap_lisp = lisp.get_lisp("keymap");
  if(keymap_lisp) {
    keymap.clear();
    keymap_lisp->get("jump-with-up", jump_with_up_kbd);
    lisp::ListIterator iter(keymap_lisp);
    while(iter.next()) {
      if(iter.item() == "map") {
        int key = -1;
        std::string control;
        const lisp::Lisp* map = iter.lisp();
        map->get("key", key);
        map->get("control", control);
//        if(key < SDLK_FIRST || key >= SDLK_LAST) {
//          log_info << "Invalid key '" << key << "' in keymap" << std::endl;
//          continue;
//        }

        int i = 0;
        for(i = 0; Controller::controlNames[i] != 0; ++i) {
          if(control == Controller::controlNames[i])
            break;
        }
        if(Controller::controlNames[i] == 0) {
          log_info << "Invalid control '" << control << "' in keymap" << std::endl;
          continue;
        }
        keymap[SDL_Keycode(key)] = Control(i);
      }
    }
  }

  const lisp::Lisp* joystick_lisp = lisp.get_lisp(_("joystick"));
  if(joystick_lisp) {
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
          bind_joybutton(0, button, Control(i));
        }

        if (map->get("axis",   axis)) {
          if (js_available && (axis == 0 || abs(axis) > max_joyaxis)) {
            log_info << "Invalid axis '" << axis << "' in axismap" << std::endl;
            continue;
          }
          bind_joyaxis(0, axis, Control(i));
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
            bind_joyhat(0, hat, Control(i));
          }
        }
      }
    }
  }
}

void
JoystickKeyboardController::write(Writer& writer)
{
  writer.start_list("keymap");
  writer.write("jump-with-up", jump_with_up_kbd);
  for(KeyMap::iterator i = keymap.begin(); i != keymap.end(); ++i) {
    writer.start_list("map");
    writer.write("key", (int) i->first);
    writer.write("control", Controller::controlNames[i->second]);
    writer.end_list("map");
  }
  writer.end_list("keymap");

  writer.start_list("joystick");
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

  writer.end_list("joystick");
}

void
JoystickKeyboardController::update()
{
  controller->update();
}

void
JoystickKeyboardController::reset()
{
  controller->reset();
}

void
JoystickKeyboardController::set_joy_controls(Control id, bool value)
{
  if (jump_with_up_joy && id == Controller::UP)
    controller->set_control(Controller::JUMP, value);

  controller->set_control(id, value);
}

void
JoystickKeyboardController::process_event(const SDL_Event& event)
{
  switch(event.type) {
    case SDL_KEYUP:
    case SDL_KEYDOWN:
      process_key_event(event.key);
      break;

    case SDL_JOYAXISMOTION:
      process_axis_event(event.jaxis);
      break;

    case SDL_JOYHATMOTION:
      process_hat_event(event.jhat);
      break;

    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
      process_button_event(event.jbutton);
      break;

    default:
      break;
  }
}

void
JoystickKeyboardController::process_button_event(const SDL_JoyButtonEvent& jbutton)
{
  if(wait_for_joystick >= 0) 
  {
    if(jbutton.state == SDL_PRESSED)
    {
      bind_joybutton(jbutton.which, jbutton.button, (Control)wait_for_joystick);
      MenuStorage::get_joystick_options_menu()->update();
      reset();
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

void
JoystickKeyboardController::process_axis_event(const SDL_JoyAxisEvent& jaxis)
{
  if (wait_for_joystick >= 0)
  {
    if (abs(jaxis.value) > dead_zone) {
      if (jaxis.value < 0)
        bind_joyaxis(jaxis.which, -(jaxis.axis + 1), Control(wait_for_joystick));
      else
        bind_joyaxis(jaxis.which, jaxis.axis + 1, Control(wait_for_joystick));

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
JoystickKeyboardController::process_hat_event(const SDL_JoyHatEvent& jhat)
{
  Uint8 changed = hat_state ^ jhat.value;

  if (wait_for_joystick >= 0)
  {
    if (changed & SDL_HAT_UP && jhat.value & SDL_HAT_UP)
      bind_joyhat(jhat.which, SDL_HAT_UP, Control(wait_for_joystick));

    if (changed & SDL_HAT_DOWN && jhat.value & SDL_HAT_DOWN)
      bind_joyhat(jhat.which, SDL_HAT_DOWN, Control(wait_for_joystick));

    if (changed & SDL_HAT_LEFT && jhat.value & SDL_HAT_LEFT)
      bind_joyhat(jhat.which, SDL_HAT_LEFT, Control(wait_for_joystick));

    if (changed & SDL_HAT_RIGHT && jhat.value & SDL_HAT_RIGHT)
      bind_joyhat(jhat.which, SDL_HAT_RIGHT, Control(wait_for_joystick));

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
JoystickKeyboardController::process_key_event(const SDL_KeyboardEvent& event)
{
  KeyMap::iterator key_mapping = keymap.find(event.keysym.sym);

  // if console key was pressed: toggle console
  if ((key_mapping != keymap.end()) && (key_mapping->second == Controller::CONSOLE)) {
    if (event.type == SDL_KEYDOWN) 
      Console::instance->toggle();
  } else {
    if (Console::instance->hasFocus()) {
      // if console is open: send key there
      process_console_key_event(event);
    } else if (MenuManager::current()) {
      // if menu mode: send key there
      process_menu_key_event(event);
    } else if(key_mapping == keymap.end()) {
      // default action: update controls
      //log_debug << "Key " << event.key.SDL_Keycode.sym << " is unbound" << std::endl;
    } else {
      Control control = key_mapping->second;
      bool value = (event.type == SDL_KEYDOWN);
      controller->set_control(control, value);
      if (jump_with_up_kbd && control == Controller::UP){
        controller->set_control(Controller::JUMP, value);
      }
    }
  }
}

void
JoystickKeyboardController::process_console_key_event(const SDL_KeyboardEvent& event)
{
  if (event.type != SDL_KEYDOWN) return;

  switch (event.keysym.sym) {
    case SDLK_RETURN:
      Console::instance->enter();
      break;
    case SDLK_BACKSPACE:
      Console::instance->backspace();
      break;
    case SDLK_TAB:
      Console::instance->autocomplete();
      break;
    case SDLK_PAGEUP:
      Console::instance->scroll(-1);
      break;
    case SDLK_PAGEDOWN:
      Console::instance->scroll(+1);
      break;
    case SDLK_HOME:
      Console::instance->move_cursor(-65535);
      break;
    case SDLK_END:
      Console::instance->move_cursor(+65535);
      break;
    case SDLK_UP:
      Console::instance->show_history(-1);
      break;
    case SDLK_DOWN:
      Console::instance->show_history(+1);
      break;
    case SDLK_LEFT:
      Console::instance->move_cursor(-1);
      break;
    case SDLK_RIGHT:
      Console::instance->move_cursor(+1);
      break;
    default:
   //   int c = SDL_GetScancodeName(event.keysym);
 //     if ((c >= 32) && (c <= 126)) { <Xeek> you need to move that "unicode" source we were originaly talkinga bout into a new function that gets called from the SDL_TextInput event..... you'll be adding that event.
   //     Console::instance->input((char)c);
     // }
      break;
  }
}

void
JoystickKeyboardController::process_menu_key_event(const SDL_KeyboardEvent& event)
{
  // wait for key mode?
  if(wait_for_key >= 0) {
    if(event.type == SDL_KEYUP)
      return;

    if(event.keysym.sym != SDLK_ESCAPE
       && event.keysym.sym != SDLK_PAUSE) {
      bind_key(event.keysym.sym, Control(wait_for_key));
    }
    reset();
    MenuStorage::get_key_options_menu()->update();
    wait_for_key = -1;
    return;
  }
  if(wait_for_joystick >= 0) {
    if(event.keysym.sym == SDLK_ESCAPE) {
      reset();
      MenuStorage::get_joystick_options_menu()->update();
      wait_for_joystick = -1;
    }
    return;
  }

  Control control;
  /* we use default keys when the menu is open (to avoid problems when
   * redefining keys to invalid settings
   */
  switch(event.keysym.sym) {
    case SDLK_UP:
      control = Controller::UP;
      break;
    case SDLK_DOWN:
      control = Controller::DOWN;
      break;
    case SDLK_LEFT:
      control = Controller::LEFT;
      break;
    case SDLK_RIGHT:
      control = Controller::RIGHT;
      break;
    case SDLK_SPACE:
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
      control = Controller::MENU_SELECT;
      break;
    case SDLK_ESCAPE:
    case SDLK_PAUSE:
      control = Controller::PAUSE_MENU;
      break;
    default:
      return;
      break;
  }

  controller->set_control(control, (event.type == SDL_KEYDOWN));
}

void
JoystickKeyboardController::unbind_joystick_control(Control control)
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
JoystickKeyboardController::bind_joyaxis(JoyId joy_id, int axis, Control control)
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
JoystickKeyboardController::bind_joyhat(JoyId joy_id, int dir, Control c)
{
  unbind_joystick_control(c);

  // add new mapping
  joy_hat_map[std::make_pair(joy_id, dir)] = c;
}

void
JoystickKeyboardController::bind_joybutton(JoyId joy_id, int button, Control control)
{
  unbind_joystick_control(control);

  // add new mapping
  joy_button_map[std::make_pair(joy_id, button)] = control;
}

void
JoystickKeyboardController::bind_key(SDL_Keycode key, Control control)
{
  // remove all previous mappings for that control and for that key
  for(KeyMap::iterator i = keymap.begin();
      i != keymap.end(); /* no ++i */) {
    if(i->second == control) {
      KeyMap::iterator e = i;
      ++i;
      keymap.erase(e);
    } else {
      ++i;
    }
  }

  KeyMap::iterator i = keymap.find(key);
  if(i != keymap.end())
    keymap.erase(i);

  // add new mapping
  keymap[key] = control;
}

void
JoystickKeyboardController::print_joystick_mappings()
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

SDL_Keycode
JoystickKeyboardController::reversemap_key(Control c)
{
  for(KeyMap::iterator i = keymap.begin(); i != keymap.end(); ++i) {
    if(i->second == c)
      return i->first;
  }

  return SDLK_UNKNOWN;
}

int
JoystickKeyboardController::reversemap_joyaxis(Control c)
{
  for(AxisMap::iterator i = joy_axis_map.begin(); i != joy_axis_map.end(); ++i) {
    if(i->second == c)
      return i->first.second;
  }

  return 0;
}

int
JoystickKeyboardController::reversemap_joybutton(Control c)
{
  for(ButtonMap::iterator i = joy_button_map.begin(); i != joy_button_map.end(); ++i) {
    if(i->second == c)
      return i->first.second;
  }

  return -1;
}

int
JoystickKeyboardController::reversemap_joyhat(Control c)
{
  for(HatMap::iterator i = joy_hat_map.begin(); i != joy_hat_map.end(); ++i) {
    if(i->second == c)
      return i->first.second;
  }

  return -1;
}

/* EOF */
