//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include <config.h>

#include <sstream>
#include "joystickkeyboardcontroller.h"
#include "gui/menu.h"
#include "app/gettext.h"
#include "lisp/lisp.h"
#include "lisp/list_iterator.h"
#include "game_session.h"

class JoystickKeyboardController::JoystickMenu : public Menu
{
public:
  JoystickMenu(JoystickKeyboardController* controller);
  virtual ~JoystickMenu();

  void update();
  std::string get_button_name(int button);
  virtual void menu_action(MenuItem* item);
  JoystickKeyboardController* controller;
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
  
JoystickKeyboardController::JoystickKeyboardController()
  : wait_for_key(-1), wait_for_joybutton(-1)
{
	memset(last_keys, 0, sizeof(last_keys));

  // initialize default keyboard map
  keymap.insert(std::make_pair(SDLK_LEFT, LEFT));
  keymap.insert(std::make_pair(SDLK_RIGHT, RIGHT));
  keymap.insert(std::make_pair(SDLK_UP, UP));
  keymap.insert(std::make_pair(SDLK_DOWN, DOWN));
  keymap.insert(std::make_pair(SDLK_SPACE, JUMP));
  keymap.insert(std::make_pair(SDLK_LCTRL, ACTION));
  keymap.insert(std::make_pair(SDLK_LALT, ACTION));
  keymap.insert(std::make_pair(SDLK_ESCAPE, PAUSE_MENU));
  keymap.insert(std::make_pair(SDLK_p, PAUSE_MENU));
  keymap.insert(std::make_pair(SDLK_PAUSE, PAUSE_MENU));  
  keymap.insert(std::make_pair(SDLK_RETURN, MENU_SELECT));
  keymap.insert(std::make_pair(SDLK_KP_ENTER, MENU_SELECT));
  
  joystick_count = SDL_NumJoysticks();
  min_joybuttons = -1;
  max_joybuttons = -1;
#ifdef DEBUG
  std::cout << "Found " << joystick_count << " joysticks.\n";
#endif
  for(int i = 0; i < joystick_count; ++i) {
    SDL_Joystick* joystick = SDL_JoystickOpen(i);
    bool good = true;
    if(SDL_JoystickNumButtons(joystick) < 2) {
      std::cerr << "Joystick " << i << " has less than 2 buttons.\n";
      good = false;
    }
    if(SDL_JoystickNumAxes(joystick) < 2
       && SDL_JoystickNumHats(joystick) == 0) {
      std::cerr << "Joystick " << i << " has less than 2 axes and no hat.\n";
      good = false;
    }
    if(!good) {
      SDL_JoystickClose(joystick);
      joysticks.push_back(0);
      joystick_names.push_back("");
      continue;
    }
    
    if(min_joybuttons < 0 || SDL_JoystickNumButtons(joystick) < min_joybuttons)
      min_joybuttons = SDL_JoystickNumButtons(joystick);
    if(SDL_JoystickNumButtons(joystick) > max_joybuttons) {
      max_joybuttons = SDL_JoystickNumButtons(joystick);
    }

    joystick_names.push_back(SDL_JoystickName(i));
    joysticks.push_back(joystick);
  }

  use_hat = true;
  joyaxis_x = 0;
  joyaxis_y = 1;
  dead_zone_x = 1000;
  dead_zone_y = 1000;
  
  joy_button_map.insert(std::make_pair(0, JUMP));
  joy_button_map.insert(std::make_pair(1, ACTION));
  // map the last 2 buttons to menu and pause
  if(min_joybuttons > 2)
    joy_button_map.insert(std::make_pair(min_joybuttons-1, PAUSE_MENU));
  // map all remaining joystick buttons to MENU_SELECT
  for(int i = 2; i < max_joybuttons; ++i) {
    if(i != min_joybuttons-1)
      joy_button_map.insert(std::make_pair(i, MENU_SELECT));
  }
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
JoystickKeyboardController::read(const lisp::Lisp& lisp)
{
  const lisp::Lisp* keymap_lisp = lisp.get_lisp("keymap");
  if(keymap_lisp) {
    keymap.clear();
    lisp::ListIterator iter(keymap_lisp);
    while(iter.next()) {
      if(iter.item() == "map") {
        int key = -1;
        std::string control;
        const lisp::Lisp* map = iter.lisp();
        map->get("key", key);
        map->get("control", control);
        if(key < SDLK_FIRST || key >= SDLK_LAST) {
          std::cerr << "Invalid key '" << key << "' in keymap.\n";
          continue;
        }

        int i = 0;
        for(i = 0; controlNames[i] != 0; ++i) {
          if(control == controlNames[i])
            break;
        }
        if(controlNames[i] == 0) {
          std::cerr << "Invalid control '" << control << "' in keymap.\n";
          continue;
        }
        keymap.insert(std::make_pair((SDLKey) key, (Control) i));
      } else {
        std::cerr << "Invalid lisp element '" << iter.item() << "' in keymap.\n";
      }
    }
  }

  const lisp::Lisp* joystick_lisp = lisp.get_lisp("joystick");
  if(joystick_lisp) {
    joystick_lisp->get("use_hat", use_hat);
    joystick_lisp->get("axis_x", joyaxis_x);
    joystick_lisp->get("axis_y", joyaxis_y);
    joystick_lisp->get("dead_zone_x", dead_zone_x);
    joystick_lisp->get("dead_zone_y", dead_zone_y);
    lisp::ListIterator iter(joystick_lisp);
    while(iter.next()) {
      if(iter.item() == "map") {
        int button = -1;
        std::string control;
        const lisp::Lisp* map = iter.lisp();
        map->get("button", button);
        map->get("control", control);
        if(button < 0 || button >= max_joybuttons) {
          std::cerr << "Invalid button '" << button << "' in buttonmap.\n";
          continue;
        }
        
        int i = 0;
        for(i = 0; controlNames[i] != 0; ++i) {
          if(control == controlNames[i])
            break;
        }                                                                           
        if(controlNames[i] == 0) {
          std::cerr << "Invalid control '" << control << "' in buttonmap.\n";
          continue;
        }
        reset_joybutton(button, (Control) i);
      }
    }
  }
}

void
JoystickKeyboardController::write(lisp::Writer& writer)
{
  writer.start_list("keymap");
  for(KeyMap::iterator i = keymap.begin(); i != keymap.end(); ++i) {
    writer.start_list("map");
    writer.write_int("key", (int) i->first);
    writer.write_string("control", controlNames[i->second]);
    writer.end_list("map");
  }
  writer.end_list("keymap");
  writer.start_list("joystick");
  writer.write_bool("use_hat", use_hat);
  writer.write_int("axis_x", joyaxis_x);
  writer.write_int("axis_y", joyaxis_y);
  writer.write_int("dead_zone_x", dead_zone_x);
  writer.write_int("dead_zone_y", dead_zone_y);
  for(ButtonMap::iterator i = joy_button_map.begin(); i != joy_button_map.end();
      ++i) {
    writer.start_list("map");
    writer.write_int("button", i->first);
    writer.write_string("control", controlNames[i->second]);
    writer.end_list("map");
  }
  writer.end_list("joystick");  
}

void
JoystickKeyboardController::process_event(const SDL_Event& event)
{
  switch(event.type) {
    case SDL_KEYUP:
    case SDL_KEYDOWN:
      // remember ascii keys for cheat codes...
      if(event.type == SDL_KEYDOWN && 
          (event.key.keysym.unicode & 0xFF80) == 0) {
        memmove(last_keys, last_keys+1, sizeof(last_keys)-1);
        last_keys[sizeof(last_keys)-1] = event.key.keysym.unicode;
        if(GameSession::current())
          GameSession::current()->try_cheats();
      }
			
      // menu mode?
      if(Menu::current()) { // menu mode
        process_menu_key_event(event);
        return;
      } else {
        // normal mode, find key in keymap
        KeyMap::iterator i = keymap.find(event.key.keysym.sym);
        if(i == keymap.end()) {
#ifdef DEBUG
          std::cerr << "Pressed key without mapping.\n";
#endif
          return;
        }
        Control control = i->second;
        controls[control] = event.type == SDL_KEYDOWN ? true : false;
      }
      break;

    case SDL_JOYAXISMOTION:
      if(event.jaxis.axis == joyaxis_x) {
        if(event.jaxis.value < -dead_zone_x) {
          controls[LEFT] = true;
          controls[RIGHT] = false;
        } else if(event.jaxis.value > dead_zone_x) {
          controls[LEFT] = false;
          controls[RIGHT] = true;
        } else {
          controls[LEFT] = false;
          controls[RIGHT] = false;
        }
      } else if(event.jaxis.axis == joyaxis_y) {
        if(event.jaxis.value < -dead_zone_y) {
          controls[UP] = true;
          controls[DOWN] = false;
        } else if(event.jaxis.value > dead_zone_y) {
          controls[UP] = false;
          controls[DOWN] = true;
        } else {
          controls[UP] = false;
          controls[DOWN] = false;
        }
      }
      break;

    case SDL_JOYHATMOTION:
      if(!use_hat)
        break;
      
      if(event.jhat.value & SDL_HAT_UP) {
        controls[UP] = true;
        controls[DOWN] = false;
      }
      if(event.jhat.value & SDL_HAT_DOWN) {
        controls[UP] = false;
        controls[DOWN] = true;
      }
      if(event.jhat.value & SDL_HAT_LEFT) {
        controls[LEFT] = true;
        controls[RIGHT] = false;
      }
      if(event.jhat.value & SDL_HAT_RIGHT) {
        controls[LEFT] = false;
        controls[RIGHT] = true;
      }
      if(event.jhat.value == SDL_HAT_CENTERED) {
        controls[UP] = false;
        controls[DOWN] = false;
        controls[LEFT] = false;
        controls[RIGHT] = false;
      }
      break;

    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
    {
      if(wait_for_joybutton >= 0) {
        if(event.type == SDL_JOYBUTTONUP)
          return;

        Control c = (Control) wait_for_joybutton;
        reset_joybutton(event.jbutton.button, c);
        reset();
        joystick_options_menu->update();
        wait_for_joybutton = -1;
        return;
      }

      ButtonMap::iterator i = joy_button_map.find(event.jbutton.button);
      if(i == joy_button_map.end()) {
#ifdef DEBUG
        std::cerr << "Unmapped joybutton " << (int) event.jbutton.button
          << " pressed.\n";
#endif
        return;
      }
      
      controls[i->second] =
        event.type == SDL_JOYBUTTONDOWN ? true : false;
      break;
    }

    default:
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
      reset_key(event.key.keysym.sym, (Control) wait_for_key);
    }
    reset();
    key_options_menu->update();
    wait_for_key = -1;
    return;
  } 
  if(wait_for_joybutton >= 0) {
    if(event.key.keysym.sym == SDLK_ESCAPE) {
      reset();
      joystick_options_menu->update();
      wait_for_joybutton = -1;
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

  controls[control] = event.type == SDL_KEYDOWN ? true : false;
}

void
JoystickKeyboardController::reset_joybutton(int button, Control control)
{
  // remove all previous mappings for that control and for that key
  for(ButtonMap::iterator i = joy_button_map.begin();
      i != joy_button_map.end(); /* no ++i */) {
    if(i->second == control) {
      ButtonMap::iterator e = i;
      ++i;
      joy_button_map.erase(e);
    } else {
      ++i;
    }
  }
  ButtonMap::iterator i = joy_button_map.find(button);
  if(i != joy_button_map.end())
    joy_button_map.erase(i);

  // add new mapping
  joy_button_map.insert(std::make_pair(button, control));

  // map all unused buttons to MENU_SELECT
  for(int b = 0; b < max_joybuttons; ++b) {
    ButtonMap::iterator i = joy_button_map.find(b);
    if(i != joy_button_map.end())
      continue;

    joy_button_map.insert(std::make_pair(b, MENU_SELECT));
  }
}

void
JoystickKeyboardController::reset_key(SDLKey key, Control control)
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
  keymap.insert(std::make_pair(key, control));
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
JoystickKeyboardController::reversemap_joybutton(Control c)
{
  for(ButtonMap::iterator i = joy_button_map.begin();
      i != joy_button_map.end(); ++i) {
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

bool
JoystickKeyboardController::check_cheatcode(const std::string& cheatcode)
{
  if(cheatcode.size() > sizeof(last_keys)) {
#ifdef DEBUG
    std::cerr << "Cheat Code too long.\n";
#endif
    return false;
  }

  for(size_t i = 0; i < cheatcode.size(); ++i) {
    if(last_keys[sizeof(last_keys)-1 - i] != cheatcode[cheatcode.size()-1-i])
      return false;
  }
  return true;
}

//----------------------------------------------------------------------------

JoystickKeyboardController::KeyboardMenu::KeyboardMenu(
    JoystickKeyboardController* _controller)
  : controller(_controller)
{
    add_label(_("Keyboard Setup"));
    add_hl();
    add_controlfield(Controller::UP, _("Up"));
    add_controlfield(Controller::DOWN, _("Down"));
    add_controlfield(Controller::LEFT, _("Left"));
    add_controlfield(Controller::RIGHT, _("Right"));
    add_controlfield(Controller::JUMP, _("Jump"));
    add_controlfield(Controller::ACTION, _("Shoot/Run"));
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
  assert(item->id >= 0 && item->id < Controller::CONTROLCOUNT);
  item->change_input(_("Press Key"));
  controller->wait_for_key = item->id;
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
}

//---------------------------------------------------------------------------

JoystickKeyboardController::JoystickMenu::JoystickMenu(
  JoystickKeyboardController* _controller)
  : controller(_controller)
{
  add_label(_("Joystick Setup"));
  add_hl();
  add_controlfield(Controller::JUMP, _("Jump"));
  add_controlfield(Controller::ACTION, _("Shoot/Run"));
  add_controlfield(Controller::PAUSE_MENU, _("Pause/Menu"));
  add_hl();
  add_back(_("Back"));
  update();
}

JoystickKeyboardController::JoystickMenu::~JoystickMenu()
{}

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
  assert(item->id >= 0 && item->id < Controller::CONTROLCOUNT);
  item->change_input(_("Press Button"));
  controller->wait_for_joybutton = item->id;
}

void
JoystickKeyboardController::JoystickMenu::update()
{
  // update menu
  get_item_by_id((int) Controller::JUMP).change_input(get_button_name(
    controller->reversemap_joybutton(Controller::JUMP)));
  get_item_by_id((int) Controller::ACTION).change_input(get_button_name(
    controller->reversemap_joybutton(Controller::ACTION)));
  get_item_by_id((int) Controller::PAUSE_MENU).change_input(get_button_name(
    controller->reversemap_joybutton(Controller::PAUSE_MENU)));
}

