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

#include "gui/menu.hpp"
#include "util/writer.hpp"
#include "lisp/list_iterator.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/console.hpp"
#include "util/gettext.hpp"

namespace{
  const int SCAN_JOYSTICKS = Controller::CONTROLCOUNT + 1;
}

class JoystickKeyboardController::JoystickMenu : public Menu
{
public:
  JoystickMenu(JoystickKeyboardController* controller);
  virtual ~JoystickMenu();

  void update();
  std::string get_button_name(int button);
  void update_menu_item(Control id);
  virtual void menu_action(MenuItem* item);
  JoystickKeyboardController* controller;
private:
  void recreateMenu();
};

class JoystickKeyboardController::KeyboardMenu : public Menu
{
public:
  KeyboardMenu(JoystickKeyboardController* controller);
  ~KeyboardMenu();

  void update();
  std::string get_key_name(SDLKey key);
  virtual void menu_action(MenuItem* item);
  JoystickKeyboardController* controller;
};

JoystickKeyboardController::JoystickKeyboardController() :
  hat_state(0),
  wait_for_key(-1), 
  wait_for_joystick(-1),
  key_options_menu(0), 
  joystick_options_menu(0)
{
  // initialize default keyboard map
  keymap[SDLK_LEFT]     = LEFT;
  keymap[SDLK_RIGHT]    = RIGHT;
  keymap[SDLK_UP]       = UP;
  keymap[SDLK_DOWN]     = DOWN;
  keymap[SDLK_SPACE]    = JUMP;
  keymap[SDLK_LCTRL]    = ACTION;
  keymap[SDLK_LALT]     = ACTION;
  keymap[SDLK_ESCAPE]   = PAUSE_MENU;
  keymap[SDLK_p]        = PAUSE_MENU;
  keymap[SDLK_PAUSE]    = PAUSE_MENU;
  keymap[SDLK_RETURN]   = MENU_SELECT;
  keymap[SDLK_KP_ENTER] = MENU_SELECT;
  keymap[SDLK_CARET]    = CONSOLE;
  keymap[SDLK_DELETE]   = PEEK_LEFT;
  keymap[SDLK_PAGEDOWN] = PEEK_RIGHT;
  keymap[SDLK_HOME]     = PEEK_UP;
  keymap[SDLK_END]      = PEEK_DOWN;

  jump_with_up_joy = false;
  jump_with_up_kbd = false;

  updateAvailableJoysticks();

  dead_zone = 1000;

  // Default joystick button configuration
  joy_button_map[0] = JUMP;
  joy_button_map[1] = ACTION;
  // 6 or more Buttons
  if( min_joybuttons > 5 ){
    joy_button_map[4] = PEEK_LEFT;
    joy_button_map[5] = PEEK_RIGHT;
    // 8 or more
    if(min_joybuttons > 7)
      joy_button_map[min_joybuttons-1] = PAUSE_MENU;
  } else {
    // map the last 2 buttons to menu and pause
    if(min_joybuttons > 2)
      joy_button_map[min_joybuttons-1] = PAUSE_MENU;
    // map all remaining joystick buttons to MENU_SELECT
    for(int i = 2; i < max_joybuttons; ++i) {
      if(i != min_joybuttons-1)
        joy_button_map[i] = MENU_SELECT;
    }
  }

  // Default joystick axis configuration
  joy_axis_map[-1] = LEFT;
  joy_axis_map[ 1] = RIGHT;
  joy_axis_map[-2] = UP;
  joy_axis_map[ 2] = DOWN;
}

JoystickKeyboardController::~JoystickKeyboardController()
{
  for(std::vector<SDL_Joystick*>::iterator i = joysticks.begin();
      i != joysticks.end(); ++i) {
    if(*i != 0)
      SDL_JoystickClose(*i);
  }

  delete key_options_menu;
  delete joystick_options_menu;
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
        log_info << "Joystick " << i << ": " << SDL_JoystickName(i) << " has less than 2 buttons" << std::endl;
        good = false;
      }
      if(SDL_JoystickNumAxes(joystick) < 2
         && SDL_JoystickNumHats(joystick) == 0) {
        log_info << "Joystick " << i << ": " << SDL_JoystickName(i) << " has less than 2 axes and no hat" << std::endl;
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
        if(key < SDLK_FIRST || key >= SDLK_LAST) {
          log_info << "Invalid key '" << key << "' in keymap" << std::endl;
          continue;
        }

        int i = 0;
        for(i = 0; controlNames[i] != 0; ++i) {
          if(control == controlNames[i])
            break;
        }
        if(controlNames[i] == 0) {
          log_info << "Invalid control '" << control << "' in keymap" << std::endl;
          continue;
        }
        keymap[(SDLKey) key] = (Control)i;
      }
    }
  }

  const lisp::Lisp* joystick_lisp = lisp.get_lisp("joystick");
  if(joystick_lisp) {
    joystick_lisp->get("dead-zone", dead_zone);
    joystick_lisp->get("jump-with-up", jump_with_up_joy);
    lisp::ListIterator iter(joystick_lisp);
    while(iter.next()) {
      if(iter.item() == "map") {
        int button = -1;
        int axis   = 0;
        int hat    = -1;
        std::string control;
        const lisp::Lisp* map = iter.lisp();

        map->get("control", control);
        int i = 0;
        for(i = 0; controlNames[i] != 0; ++i) {
          if(control == controlNames[i])
            break;
        }
        if(controlNames[i] == 0) {
          log_info << "Invalid control '" << control << "' in buttonmap" << std::endl;
          continue;
        }

        if (map->get("button", button)) {
          if(button < 0 || button >= max_joybuttons) {
            log_info << "Invalid button '" << button << "' in buttonmap" << std::endl;
            continue;
          }
          bind_joybutton(button, (Control) i);
        }

        if (map->get("axis",   axis)) {
          if (axis == 0 || abs(axis) > max_joyaxis) {
            log_info << "Invalid axis '" << axis << "' in axismap" << std::endl;
            continue;
          }
          bind_joyaxis(axis, (Control) i);
        }

        if (map->get("hat",   hat)) {
          if (hat != SDL_HAT_UP   &&
              hat != SDL_HAT_DOWN &&
              hat != SDL_HAT_LEFT &&
              hat != SDL_HAT_RIGHT) {
            log_info << "Invalid axis '" << axis << "' in axismap" << std::endl;
            continue;
          } else {
            bind_joyhat(hat, (Control) i);
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
    writer.write("control", controlNames[i->second]);
    writer.end_list("map");
  }
  writer.end_list("keymap");

  writer.start_list("joystick");
  writer.write("dead-zone", dead_zone);
  writer.write("jump-with-up", jump_with_up_joy);

  for(ButtonMap::iterator i = joy_button_map.begin(); i != joy_button_map.end();
      ++i) {
    writer.start_list("map");
    writer.write("button", i->first);
    writer.write("control", controlNames[i->second]);
    writer.end_list("map");
  }

  for(HatMap::iterator i = joy_hat_map.begin(); i != joy_hat_map.end(); ++i) {
    writer.start_list("map");
    writer.write("hat", i->first);
    writer.write("control", controlNames[i->second]);
    writer.end_list("map");
  }

  for(AxisMap::iterator i = joy_axis_map.begin(); i != joy_axis_map.end(); ++i) {
    writer.start_list("map");
    writer.write("axis", i->first);
    writer.write("control", controlNames[i->second]);
    writer.end_list("map");
  }

  writer.end_list("joystick");
}

void
JoystickKeyboardController::reset()
{
  Controller::reset();
}

void
JoystickKeyboardController::set_joy_controls(Control id, bool value)
{
  if (jump_with_up_joy && id == Controller::UP)
    controls[Controller::JUMP] = value;

  controls[(Control)id] = value;
}

void
JoystickKeyboardController::process_event(const SDL_Event& event)
{
  switch(event.type) {
    case SDL_KEYUP:
    case SDL_KEYDOWN:
      process_key_event(event);
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
      bind_joybutton(jbutton.button, (Control)wait_for_joystick);
      joystick_options_menu->update();
      reset();
      wait_for_joystick = -1;
    }
  } 
  else 
  {
    ButtonMap::iterator i = joy_button_map.find(jbutton.button);
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
        bind_joyaxis(-(jaxis.axis + 1), Control(wait_for_joystick));
      else
        bind_joyaxis(jaxis.axis + 1, Control(wait_for_joystick));

      joystick_options_menu->update();
      wait_for_joystick = -1;
    }
  }
  else
  {
    // Split the axis into left and right, so that both can be
    // mapped separately (needed for jump/down vs up/down)
    int axis = jaxis.axis + 1;

    AxisMap::iterator left  = joy_axis_map.find(-axis);
    AxisMap::iterator right = joy_axis_map.find(axis);

    if(left == joy_axis_map.end()) {
      std::cout << "Unmapped joyaxis " << (int)jaxis.axis << " moved" << std::endl;
    } else {
      if (jaxis.value < -dead_zone)
        set_joy_controls(left->second,  true);
      else if (jaxis.value > dead_zone)
        set_joy_controls(left->second, false);
      else
        set_joy_controls(left->second, false);
    }

    if(right == joy_axis_map.end()) {
      std::cout << "Unmapped joyaxis " << (int)jaxis.axis << " moved" << std::endl;
    } else {
      if (jaxis.value < -dead_zone)
        set_joy_controls(right->second, false);
      else if (jaxis.value > dead_zone)
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
      bind_joyhat(SDL_HAT_UP, (Control)wait_for_joystick);

    if (changed & SDL_HAT_DOWN && jhat.value & SDL_HAT_DOWN)
      bind_joyhat(SDL_HAT_DOWN, (Control)wait_for_joystick);

    if (changed & SDL_HAT_LEFT && jhat.value & SDL_HAT_LEFT)
      bind_joyhat(SDL_HAT_LEFT, (Control)wait_for_joystick);

    if (changed & SDL_HAT_RIGHT && jhat.value & SDL_HAT_RIGHT)
      bind_joyhat(SDL_HAT_RIGHT, (Control)wait_for_joystick);

    joystick_options_menu->update();
    wait_for_joystick = -1;
  }
  else
  {
    if (changed & SDL_HAT_UP)
    {
      HatMap::iterator it = joy_hat_map.find(SDL_HAT_UP);
      if (it != joy_hat_map.end())
        set_joy_controls(it->second, jhat.value & SDL_HAT_UP);
    }

    if (changed & SDL_HAT_DOWN)
    {
      HatMap::iterator it = joy_hat_map.find(SDL_HAT_DOWN);
      if (it != joy_hat_map.end())
        set_joy_controls(it->second, jhat.value & SDL_HAT_DOWN);
    }

    if (changed & SDL_HAT_LEFT)
    {
      HatMap::iterator it = joy_hat_map.find(SDL_HAT_LEFT);
      if (it != joy_hat_map.end())
        set_joy_controls(it->second, jhat.value & SDL_HAT_LEFT);
    }

    if (changed & SDL_HAT_RIGHT)
    {
      HatMap::iterator it = joy_hat_map.find(SDL_HAT_RIGHT);
      if (it != joy_hat_map.end())
        set_joy_controls(it->second, jhat.value & SDL_HAT_RIGHT);
    }
  }

  hat_state = jhat.value;
}

void
JoystickKeyboardController::process_key_event(const SDL_Event& event)
{
  KeyMap::iterator key_mapping = keymap.find(event.key.keysym.sym);

  // if console key was pressed: toggle console
  if ((key_mapping != keymap.end()) && (key_mapping->second == CONSOLE)) {
    if (event.type == SDL_KEYDOWN) 
      Console::instance->toggle();
  } else {
    if (Console::instance->hasFocus()) {
      // if console is open: send key there
      process_console_key_event(event);
    } else if (Menu::current()) {
      // if menu mode: send key there
      process_menu_key_event(event);
    } else if(key_mapping == keymap.end()) {
      // default action: update controls
      //log_debug << "Key " << event.key.keysym.sym << " is unbound" << std::endl;
    } else {
      Control control = key_mapping->second;
      controls[control] = (event.type == SDL_KEYDOWN);
      if (jump_with_up_kbd && control == UP){
        controls[JUMP] = (event.type == SDL_KEYDOWN);
      }
    }
  }
}

void
JoystickKeyboardController::process_console_key_event(const SDL_Event& event)
{
  if (event.type != SDL_KEYDOWN) return;

  switch (event.key.keysym.sym) {
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
      int c = event.key.keysym.unicode;
      if ((c >= 32) && (c <= 126)) {
        Console::instance->input((char)c);
      }
      break;
  }
}

void
JoystickKeyboardController::process_menu_key_event(const SDL_Event& event)
{
  // wait for key mode?
  if(wait_for_key >= 0) {
    if(event.type == SDL_KEYUP)
      return;

    if(event.key.keysym.sym != SDLK_ESCAPE
       && event.key.keysym.sym != SDLK_PAUSE) {
      bind_key(event.key.keysym.sym, (Control) wait_for_key);
    }
    reset();
    key_options_menu->update();
    wait_for_key = -1;
    return;
  }
  if(wait_for_joystick >= 0) {
    if(event.key.keysym.sym == SDLK_ESCAPE) {
      reset();
      joystick_options_menu->update();
      wait_for_joystick = -1;
    }
    return;
  }

  Control control;
  /* we use default keys when the menu is open (to avoid problems when
   * redefining keys to invalid settings
   */
  switch(event.key.keysym.sym) {
    case SDLK_UP:
      control = UP;
      break;
    case SDLK_DOWN:
      control = DOWN;
      break;
    case SDLK_LEFT:
      control = LEFT;
      break;
    case SDLK_RIGHT:
      control = RIGHT;
      break;
    case SDLK_SPACE:
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
      control = MENU_SELECT;
      break;
    case SDLK_ESCAPE:
    case SDLK_PAUSE:
      control = PAUSE_MENU;
      break;
    default:
      return;
      break;
  }

  controls[control] = (event.type == SDL_KEYDOWN);
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
JoystickKeyboardController::bind_joyaxis(int axis, Control control)
{
  // axis isn't the SDL axis number, but axisnumber + 1 with sign
  // changed depending on if the positive or negative end is to be
  // used (negative axis 0 becomes -1, positive axis 2 becomes +3,
  // etc.)

  unbind_joystick_control(control);

  // add new mapping
  joy_axis_map[axis] = control;
}

void
JoystickKeyboardController::bind_joyhat(int dir, Control c)
{
  unbind_joystick_control(c);

  // add new mapping
  joy_hat_map[dir] = c;
}

void
JoystickKeyboardController::bind_joybutton(int button, Control control)
{
  unbind_joystick_control(control);

  // add new mapping
  joy_button_map[button] = control;
}

void
JoystickKeyboardController::bind_key(SDLKey key, Control control)
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
  keymap[key]= control;
}

void
JoystickKeyboardController::print_joystick_mappings()
{
  std::cout << "Joystick Mappings" << std::endl;
  std::cout << "-----------------" << std::endl;
  for(AxisMap::iterator i = joy_axis_map.begin(); i != joy_axis_map.end(); ++i) {
    std::cout << "Axis: " << i->first << " -> " << i->second << std::endl;
  }

  for(ButtonMap::iterator i = joy_button_map.begin(); i != joy_button_map.end(); ++i) {
    std::cout << "Button: " << i->first << " -> " << i->second << std::endl;
  }

  for(HatMap::iterator i = joy_hat_map.begin(); i != joy_hat_map.end(); ++i) {
    std::cout << "Hat: " << i->first << " -> " << i->second << std::endl;
  }
  std::cout << std::endl;
}

SDLKey
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
      return i->first;
  }

  return 0;
}

int
JoystickKeyboardController::reversemap_joybutton(Control c)
{
  for(ButtonMap::iterator i = joy_button_map.begin(); i != joy_button_map.end(); ++i) {
    if(i->second == c)
      return i->first;
  }

  return -1;
}

int
JoystickKeyboardController::reversemap_joyhat(Control c)
{
  for(HatMap::iterator i = joy_hat_map.begin(); i != joy_hat_map.end(); ++i) {
    if(i->second == c)
      return i->first;
  }

  return -1;
}

Menu*
JoystickKeyboardController::get_key_options_menu()
{
  if(key_options_menu == 0) {
    key_options_menu = new KeyboardMenu(this);
  }

  return key_options_menu;
}

Menu*
JoystickKeyboardController::get_joystick_options_menu()
{
  if(joystick_options_menu == 0) {
    joystick_options_menu = new JoystickMenu(this);
  }

  return joystick_options_menu;
}

//----------------------------------------------------------------------------

JoystickKeyboardController::KeyboardMenu::KeyboardMenu(
  JoystickKeyboardController* _controller)
  : controller(_controller)
{
  add_label(_("Setup Keyboard"));
  add_hl();
  add_controlfield(Controller::UP,         _("Up"));
  add_controlfield(Controller::DOWN,       _("Down"));
  add_controlfield(Controller::LEFT,       _("Left"));
  add_controlfield(Controller::RIGHT,      _("Right"));
  add_controlfield(Controller::JUMP,       _("Jump"));
  add_controlfield(Controller::ACTION,     _("Action"));
  add_controlfield(Controller::PEEK_LEFT,  _("Peek Left"));
  add_controlfield(Controller::PEEK_RIGHT, _("Peek Right"));
  add_controlfield(Controller::PEEK_UP,    _("Peek Up"));
  add_controlfield(Controller::PEEK_DOWN,  _("Peek Down"));
  if (g_config->console_enabled) {
    add_controlfield(Controller::CONSOLE, _("Console"));
  }
  add_toggle(Controller::CONTROLCOUNT, _("Jump with Up"), controller->jump_with_up_kbd);
  add_hl();
  add_back(_("Back"));
  update();
}

JoystickKeyboardController::KeyboardMenu::~KeyboardMenu()
{}

std::string
JoystickKeyboardController::KeyboardMenu::get_key_name(SDLKey key)
{
  switch(key) {
    case SDLK_UNKNOWN:
      return _("None");
    case SDLK_UP:
      return _("Up cursor");
    case SDLK_DOWN:
      return _("Down cursor");
    case SDLK_LEFT:
      return _("Left cursor");
    case SDLK_RIGHT:
      return _("Right cursor");
    case SDLK_RETURN:
      return _("Return");
    case SDLK_SPACE:
      return _("Space");
    case SDLK_RSHIFT:
      return _("Right Shift");
    case SDLK_LSHIFT:
      return _("Left Shift");
    case SDLK_RCTRL:
      return _("Right Control");
    case SDLK_LCTRL:
      return _("Left Control");
    case SDLK_RALT:
      return _("Right Alt");
    case SDLK_LALT:
      return _("Left Alt");
    default:
      return SDL_GetKeyName((SDLKey) key);
  }
}

void
JoystickKeyboardController::KeyboardMenu::menu_action(MenuItem* item)
{
  if(item->id >= 0 && item->id < Controller::CONTROLCOUNT){
    item->change_input(_("Press Key"));
    controller->wait_for_key = item->id;
  } else if( item->id == Controller::CONTROLCOUNT) {
    controller->jump_with_up_kbd = item->toggled;
  } 
}

void
JoystickKeyboardController::KeyboardMenu::update()
{
  // update menu
  get_item_by_id((int) Controller::UP).change_input(get_key_name(
                                                      controller->reversemap_key(Controller::UP)));
  get_item_by_id((int) Controller::DOWN).change_input(get_key_name(
                                                        controller->reversemap_key(Controller::DOWN)));
  get_item_by_id((int) Controller::LEFT).change_input(get_key_name(
                                                        controller->reversemap_key(Controller::LEFT)));
  get_item_by_id((int) Controller::RIGHT).change_input(get_key_name(
                                                         controller->reversemap_key(Controller::RIGHT)));
  get_item_by_id((int) Controller::JUMP).change_input(get_key_name(
                                                        controller->reversemap_key(Controller::JUMP)));
  get_item_by_id((int) Controller::ACTION).change_input(get_key_name(
                                                          controller->reversemap_key(Controller::ACTION)));
  get_item_by_id((int) Controller::PEEK_LEFT).change_input(get_key_name(
                                                             controller->reversemap_key(Controller::PEEK_LEFT)));
  get_item_by_id((int) Controller::PEEK_RIGHT).change_input(get_key_name(
                                                              controller->reversemap_key(Controller::PEEK_RIGHT)));
  get_item_by_id((int) Controller::PEEK_UP).change_input(get_key_name(
                                                           controller->reversemap_key(Controller::PEEK_UP)));
  get_item_by_id((int) Controller::PEEK_DOWN).change_input(get_key_name(
                                                             controller->reversemap_key(Controller::PEEK_DOWN)));
  if (g_config->console_enabled) {
    get_item_by_id((int) Controller::CONSOLE).change_input(get_key_name(
                                                             controller->reversemap_key(Controller::CONSOLE)));
  }
  get_item_by_id(Controller::CONTROLCOUNT).toggled = controller->jump_with_up_kbd;
}

//---------------------------------------------------------------------------

JoystickKeyboardController::JoystickMenu::JoystickMenu(
  JoystickKeyboardController* _controller)
  : controller(_controller)
{
  recreateMenu();
}

JoystickKeyboardController::JoystickMenu::~JoystickMenu()
{}

void
JoystickKeyboardController::JoystickMenu::recreateMenu()
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
JoystickKeyboardController::JoystickMenu::get_button_name(int button)
{
  if(button < 0)
    return _("None");

  std::ostringstream name;
  name << "Button " << button;
  return name.str();
}

void
JoystickKeyboardController::JoystickMenu::menu_action(MenuItem* item)
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
JoystickKeyboardController::JoystickMenu::update_menu_item(Control id)
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
JoystickKeyboardController::JoystickMenu::update()
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
