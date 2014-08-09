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

#include "supertux/menu/keyboard_menu.hpp"

#include "control/keyboard_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/gettext.hpp"

KeyboardMenu::KeyboardMenu(InputManager* _controller) :
  controller(_controller)
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
  add_toggle(Controller::CONTROLCOUNT, _("Jump with Up"), controller->keyboard_manager->jump_with_up_kbd);
  add_hl();
  add_back(_("Back"));
  refresh();
}

KeyboardMenu::~KeyboardMenu()
{}

std::string
KeyboardMenu::get_key_name(SDL_Keycode key)
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
      return SDL_GetKeyName((SDL_Keycode) key);
  }
}

void
KeyboardMenu::menu_action(MenuItem* item)
{
  if(item->id >= 0 && item->id < Controller::CONTROLCOUNT){
    item->change_input(_("Press Key"));
    controller->keyboard_manager->wait_for_key = item->id;
  } else if( item->id == Controller::CONTROLCOUNT) {
    controller->keyboard_manager->jump_with_up_kbd = item->toggled;
  } 
}

void
KeyboardMenu::refresh()
{
  auto& kbd_mgr = controller->keyboard_manager;

  get_item_by_id((int) Controller::UP).change_input(get_key_name(
                                                      kbd_mgr->reversemap_key(Controller::UP)));
  get_item_by_id((int) Controller::DOWN).change_input(get_key_name(
                                                        kbd_mgr->reversemap_key(Controller::DOWN)));
  get_item_by_id((int) Controller::LEFT).change_input(get_key_name(
                                                        kbd_mgr->reversemap_key(Controller::LEFT)));
  get_item_by_id((int) Controller::RIGHT).change_input(get_key_name(
                                                         kbd_mgr->reversemap_key(Controller::RIGHT)));
  get_item_by_id((int) Controller::JUMP).change_input(get_key_name(
                                                        kbd_mgr->reversemap_key(Controller::JUMP)));
  get_item_by_id((int) Controller::ACTION).change_input(get_key_name(
                                                          kbd_mgr->reversemap_key(Controller::ACTION)));
  get_item_by_id((int) Controller::PEEK_LEFT).change_input(get_key_name(
                                                             kbd_mgr->reversemap_key(Controller::PEEK_LEFT)));
  get_item_by_id((int) Controller::PEEK_RIGHT).change_input(get_key_name(
                                                              kbd_mgr->reversemap_key(Controller::PEEK_RIGHT)));
  get_item_by_id((int) Controller::PEEK_UP).change_input(get_key_name(
                                                           kbd_mgr->reversemap_key(Controller::PEEK_UP)));
  get_item_by_id((int) Controller::PEEK_DOWN).change_input(get_key_name(
                                                             kbd_mgr->reversemap_key(Controller::PEEK_DOWN)));
  if (g_config->console_enabled) {
    get_item_by_id((int) Controller::CONSOLE).change_input(get_key_name(
                                                             kbd_mgr->reversemap_key(Controller::CONSOLE)));
  }
  get_item_by_id(Controller::CONTROLCOUNT).toggled = kbd_mgr->jump_with_up_kbd;
}

/* EOF */
