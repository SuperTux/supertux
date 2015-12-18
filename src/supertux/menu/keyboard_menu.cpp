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

#include "supertux/menu/keyboard_menu.hpp"

#include "control/keyboard_manager.hpp"
#include "gui/item_controlfield.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/gettext.hpp"

KeyboardMenu::KeyboardMenu(InputManager& input_manager) :
  m_input_manager(input_manager)
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
  if (g_config->developer_mode) {
    add_controlfield(Controller::CONSOLE, _("Console"));
  }
  if (g_config->developer_mode) {
    add_controlfield(Controller::CHEAT_MENU, _("Cheat Menu"));
  }
  add_toggle(Controller::CONTROLCOUNT, _("Jump with Up"), &g_config->keyboard_config.jump_with_up_kbd);
  add_hl();
  add_back(_("Back"));
  refresh();
}

KeyboardMenu::~KeyboardMenu()
{}

std::string
KeyboardMenu::get_key_name(SDL_Keycode key) const
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
    case SDLK_RGUI:
      return _("Right Command");
    case SDLK_LGUI:
      return _("Left Command");
    default:
      return SDL_GetKeyName(static_cast<SDL_Keycode>(key));
  }
}

void
KeyboardMenu::menu_action(MenuItem* item)
{
  if(item->id >= 0 && item->id < Controller::CONTROLCOUNT){
    ItemControlField* itemcf = dynamic_cast<ItemControlField*>(item);
    if (!itemcf) {
      return;
    }
    itemcf->change_input(_("Press Key"));
    m_input_manager.keyboard_manager->bind_next_event_to(static_cast<Controller::Control>(item->id));
  }
}

void
KeyboardMenu::refresh()
{
  KeyboardConfig& kbd_cfg = g_config->keyboard_config;
  ItemControlField* micf;

  micf = dynamic_cast<ItemControlField*>(&get_item_by_id((int) Controller::UP));
  if (micf) micf->change_input(get_key_name(kbd_cfg.reversemap_key(Controller::UP)));
  micf = dynamic_cast<ItemControlField*>(&get_item_by_id((int) Controller::DOWN));
  if (micf) micf->change_input(get_key_name(kbd_cfg.reversemap_key(Controller::DOWN)));
  micf = dynamic_cast<ItemControlField*>(&get_item_by_id((int) Controller::LEFT));
  if (micf) micf->change_input(get_key_name(kbd_cfg.reversemap_key(Controller::LEFT)));
  micf = dynamic_cast<ItemControlField*>(&get_item_by_id((int) Controller::RIGHT));
  if (micf) micf->change_input(get_key_name(kbd_cfg.reversemap_key(Controller::RIGHT)));
  micf = dynamic_cast<ItemControlField*>(&get_item_by_id((int) Controller::JUMP));
  if (micf) micf->change_input(get_key_name(kbd_cfg.reversemap_key(Controller::JUMP)));
  micf = dynamic_cast<ItemControlField*>(&get_item_by_id((int) Controller::ACTION));
  if (micf) micf->change_input(get_key_name(kbd_cfg.reversemap_key(Controller::ACTION)));
  micf = dynamic_cast<ItemControlField*>(&get_item_by_id((int) Controller::PEEK_LEFT));
  if (micf) micf->change_input(get_key_name(kbd_cfg.reversemap_key(Controller::PEEK_LEFT)));
  micf = dynamic_cast<ItemControlField*>(&get_item_by_id((int) Controller::PEEK_RIGHT));
  if (micf) micf->change_input(get_key_name(kbd_cfg.reversemap_key(Controller::PEEK_RIGHT)));
  micf = dynamic_cast<ItemControlField*>(&get_item_by_id((int) Controller::PEEK_UP));
  if (micf) micf->change_input(get_key_name(kbd_cfg.reversemap_key(Controller::PEEK_UP)));
  micf = dynamic_cast<ItemControlField*>(&get_item_by_id((int) Controller::PEEK_DOWN));
  if (micf) micf->change_input(get_key_name(kbd_cfg.reversemap_key(Controller::PEEK_DOWN)));

  if (g_config->developer_mode) {
    micf = dynamic_cast<ItemControlField*>(&get_item_by_id((int) Controller::CHEAT_MENU));
    if (micf) micf->change_input(get_key_name(kbd_cfg.reversemap_key(Controller::CHEAT_MENU)));
  }

  if (g_config->developer_mode) {
    micf = dynamic_cast<ItemControlField*>(&get_item_by_id((int) Controller::CONSOLE));
    if (micf) micf->change_input(get_key_name(kbd_cfg.reversemap_key(Controller::CONSOLE)));
  }
}

/* EOF */
