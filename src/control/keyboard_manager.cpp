//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>,
//                2007-2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "control/keyboard_manager.hpp"

#include "control/joystick_manager.hpp"
#include "control/input_manager.hpp"
#include "gui/menu_manager.hpp"
#include "squirrel/squirrel_virtual_machine.hpp"
#include "supertux/console.hpp"
#include "util/log.hpp"

#include <simplesquirrel/class.hpp>
#include <simplesquirrel/enum.hpp>

KeyboardManager::KeyboardManager(InputManager* parent,
                                 KeyboardConfig& keyboard_config) :
  m_parent(parent),
  m_keyboard_config(keyboard_config),
  m_wait_for_key(),
  m_lock_text_input(false)
{
}

void
KeyboardManager::process_key_event(const SDL_KeyboardEvent& event)
{
  auto key_mapping = m_keyboard_config.m_keymap.find(event.scancode);

  // if console key was pressed: toggle console
  if (key_mapping != m_keyboard_config.m_keymap.end() &&
      key_mapping->second.control == Control::CONSOLE)
  {
    if (event.type == SDL_EVENT_KEY_DOWN)
    {
      // text input gets locked between the console-key being pressed
      // and released to avoid the console-key getting interpreted as
      // text input and echoed to the console
      m_lock_text_input = true;

      Console::current()->toggle();
    }
    else if (event.type == SDL_EVENT_KEY_UP)
    {
      m_lock_text_input = false;
    }
  }
  else if (Console::current()->hasFocus())
  {
    // if console is open: send key there
    process_console_key_event(event);
  }
  else if (MenuManager::instance().is_active())
  {
    // if menu mode: send key there
    process_menu_key_event(event);
  }
  else
  {
    if (key_mapping == m_keyboard_config.m_keymap.end())
    {
      // default action: update controls
      //log_debug << "Key " << event.key.SDL_Keycode.sym << " is unbound" << std::endl;
    }
    else
    {
      auto control = key_mapping->second;
      bool value = (event.type == SDL_EVENT_KEY_DOWN);

      if (control.player >= m_parent->get_num_users())
        return;

      m_parent->get_controller(control.player).set_control(control.control, value);

      if (m_keyboard_config.m_jump_with_up_kbd && control.control == Control::UP) {
        m_parent->get_controller(control.player).set_jump_key_with_up(value);
      }
    }

    ssq::VM& ssq_vm = SquirrelVirtualMachine::current()->get_vm();
    try
    {
      if (ssq_vm.hasEntry("sector"))
      {
        std::optional<ssq::Function> function = {};
        unsigned int param_count = 0;
        
        const auto& sector_table = ssq_vm.findTable("sector");
        if (event.down && !event.repeat && sector_table.hasEntry("on_key_down"))
        {
          function = sector_table.findFunc("on_key_down");
        }
        else if(!event.down && !event.repeat && sector_table.hasEntry("on_key_up"))
        {
          function = sector_table.findFunc("on_key_up");
        }

        if (function)
        {
          param_count = function->getNumOfParams().first;
          if (param_count == 1)
          {
            ssq_vm.callFunc(*function, ssq_vm, event.key);
          }
        }

        if (key_mapping != m_keyboard_config.m_keymap.end())
        {
          auto control = key_mapping->second;
          bool control_down = (event.type == SDL_EVENT_KEY_DOWN);

          if (control_down && sector_table.hasEntry("on_control_down"))
          {
            function = sector_table.findFunc("on_control_down");
          }

          if (!control_down && sector_table.hasEntry("on_control_up"))
          {
            function = sector_table.findFunc("on_control_up");
          }

          if (function)
          {
            param_count = function->getNumOfParams().first;

            if (param_count == 2)
            {
              ssq_vm.callFunc(*function, ssq_vm, (int)control.control, (int)control.player);
            }
            else
            {
              ssq_vm.callFunc(*function, ssq_vm, (int)control.control);
            }
          }
        }
      }
    }
    catch(std::exception& ex)
    {
      log_warning << ex.what() << std::endl;
    }

  }
}

void
KeyboardManager::process_text_input_event(const SDL_TextInputEvent& event)
{
  if (!m_lock_text_input && Console::current()->hasFocus()) {
    for (int i = 0; event.text[i] != '\0'; ++i)
    {
      Console::current()->input(event.text[i]);
    }
  }
}

void
KeyboardManager::process_console_key_event(const SDL_KeyboardEvent& event)
{
  if (event.type != SDL_EVENT_KEY_DOWN) return;
  auto console = Console::current();

  switch (event.key) {
    case SDLK_RETURN:
      console->enter();
      break;
    case SDLK_BACKSPACE:
      console->backspace();
      break;
    case SDLK_DELETE:
      console->eraseChar();
      break;
    case SDLK_TAB:
      console->autocomplete();
      break;
    case SDLK_PAGEUP:
      console->scroll(-console->get_line_height() + 2);
      break;
    case SDLK_PAGEDOWN:
      console->scroll(+console->get_line_height() - 2);
      break;
    case SDLK_HOME:
      console->move_cursor(-65535);
      break;
    case SDLK_END:
      console->move_cursor(+65535);
      break;
    case SDLK_A:
      if (event.mod & SDL_KMOD_CTRL) {
        console->move_cursor(-65535);
      }
      break;
    case SDLK_E:
      if (event.mod & SDL_KMOD_CTRL) {
        console->move_cursor(+65535);
      }
      break;
    case SDLK_UP:
      console->show_history(-1);
      break;
    case SDLK_DOWN:
      console->show_history(+1);
      break;
    case SDLK_LEFT:
      console->move_cursor(-1);
      break;
    case SDLK_RIGHT:
      console->move_cursor(+1);
      break;
    default:
      break;
  }
}

void
KeyboardManager::process_menu_key_event(const SDL_KeyboardEvent& event)
{
  // wait for key mode?
  if (m_wait_for_key)
  {
    if (event.type == SDL_EVENT_KEY_UP)
      return;

    if (event.key != SDLK_ESCAPE &&
        event.key != SDLK_PAUSE)
    {
      m_keyboard_config.bind_key(event.scancode, m_wait_for_key->player, m_wait_for_key->control);
    }
    m_parent->reset();
    MenuManager::instance().refresh();
    m_wait_for_key = std::nullopt;
    return;
  }

  if (m_parent->joystick_manager->wait_for_joystick >= 0)
  {
    if (event.key == SDLK_ESCAPE)
    {
      m_parent->reset();
      MenuManager::instance().refresh();
      m_parent->joystick_manager->wait_for_joystick = -1;
    }
    return;
  }

  Control control;
  /* we use default keys when the menu is open (to avoid problems when
   * redefining keys to invalid settings
   */
  switch (event.key) {
    case SDLK_UP:
      control = Control::UP;
      break;
    case SDLK_DOWN:
      control = Control::DOWN;
      break;
    case SDLK_LEFT:
      control = Control::LEFT;
      break;
    case SDLK_RIGHT:
      control = Control::RIGHT;
      break;
    case SDLK_SPACE:
      control = Control::MENU_SELECT_SPACE;
      break;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
      control = Control::MENU_SELECT;
      break;
    case SDLK_ESCAPE:
      control = Control::ESCAPE;
      break;
    case SDLK_PAUSE:
      control = Control::START;
      break;
    case SDLK_BACKSPACE:
      control = Control::REMOVE;
      break;
    default:
      return;
  }

  // Keep empty because this is in the menu; only the first player may navigate
  m_parent->get_controller().set_control(control, (event.type == SDL_EVENT_KEY_DOWN));
}

void
KeyboardManager::bind_next_event_to(int player_id, Control id)
{
  m_wait_for_key = KeyboardConfig::PlayerControl{player_id, id};
}

void
KeyboardManager::register_class(ssq::VM& vm)
{
  if(!vm.hasEntry("Keys"))
  {
    auto keys_enum = vm.addEnum("Keys");
    keys_enum.addSlot("SDLK_UNKNOWN", SDLK_UNKNOWN); /**< 0 */
    keys_enum.addSlot("SDLK_RETURN", SDLK_RETURN); /**< '\r' */
    keys_enum.addSlot("SDLK_ESCAPE", SDLK_ESCAPE); /**< '\x1B' */
    keys_enum.addSlot("SDLK_BACKSPACE", SDLK_BACKSPACE); /**< '\b' */
    keys_enum.addSlot("SDLK_TAB", SDLK_TAB); /**< '\t' */
    keys_enum.addSlot("SDLK_SPACE", SDLK_SPACE); /**< ' ' */
    keys_enum.addSlot("SDLK_EXCLAIM", SDLK_EXCLAIM); /**< '!' */
    keys_enum.addSlot("SDLK_DBLAPOSTROPHE", SDLK_DBLAPOSTROPHE); /**< '"' */
    keys_enum.addSlot("SDLK_HASH", SDLK_HASH); /**< '#' */
    keys_enum.addSlot("SDLK_DOLLAR", SDLK_DOLLAR); /**< '$' */
    keys_enum.addSlot("SDLK_PERCENT", SDLK_PERCENT); /**< '%' */
    keys_enum.addSlot("SDLK_AMPERSAND", SDLK_AMPERSAND); /**< '&' */
    keys_enum.addSlot("SDLK_APOSTROPHE", SDLK_APOSTROPHE); /**< '\'' */
    keys_enum.addSlot("SDLK_LEFTPAREN", SDLK_LEFTPAREN); /**< '(' */
    keys_enum.addSlot("SDLK_RIGHTPAREN", SDLK_RIGHTPAREN); /**< ')' */
    keys_enum.addSlot("SDLK_ASTERISK", SDLK_ASTERISK); /**< '*' */
    keys_enum.addSlot("SDLK_PLUS", SDLK_PLUS); /**< '+' */
    keys_enum.addSlot("SDLK_COMMA", SDLK_COMMA); /**< ',' */
    keys_enum.addSlot("SDLK_MINUS", SDLK_MINUS); /**< '-' */
    keys_enum.addSlot("SDLK_PERIOD", SDLK_PERIOD); /**< '.' */
    keys_enum.addSlot("SDLK_SLASH", SDLK_SLASH); /**< '/' */
    keys_enum.addSlot("SDLK_0", SDLK_0); /**< '0' */
    keys_enum.addSlot("SDLK_1", SDLK_1); /**< '1' */
    keys_enum.addSlot("SDLK_2", SDLK_2); /**< '2' */
    keys_enum.addSlot("SDLK_3", SDLK_3); /**< '3' */
    keys_enum.addSlot("SDLK_4", SDLK_4); /**< '4' */
    keys_enum.addSlot("SDLK_5", SDLK_5); /**< '5' */
    keys_enum.addSlot("SDLK_6", SDLK_6); /**< '6' */
    keys_enum.addSlot("SDLK_7", SDLK_7); /**< '7' */
    keys_enum.addSlot("SDLK_8", SDLK_8); /**< '8' */
    keys_enum.addSlot("SDLK_9", SDLK_9); /**< '9' */
    keys_enum.addSlot("SDLK_COLON", SDLK_COLON); /**< ':' */
    keys_enum.addSlot("SDLK_SEMICOLON", SDLK_SEMICOLON); /**< ';' */
    keys_enum.addSlot("SDLK_LESS", SDLK_LESS); /**< '<' */
    keys_enum.addSlot("SDLK_EQUALS", SDLK_EQUALS); /**< '=' */
    keys_enum.addSlot("SDLK_GREATER", SDLK_GREATER); /**< '>' */
    keys_enum.addSlot("SDLK_QUESTION", SDLK_QUESTION); /**< '?' */
    keys_enum.addSlot("SDLK_AT", SDLK_AT); /**< '@' */
    keys_enum.addSlot("SDLK_LEFTBRACKET", SDLK_LEFTBRACKET); /**< '[' */
    keys_enum.addSlot("SDLK_BACKSLASH", SDLK_BACKSLASH); /**< '\\' */
    keys_enum.addSlot("SDLK_RIGHTBRACKET", SDLK_RIGHTBRACKET); /**< ']' */
    keys_enum.addSlot("SDLK_CARET", SDLK_CARET); /**< '^' */
    keys_enum.addSlot("SDLK_UNDERSCORE", SDLK_UNDERSCORE); /**< '_' */
    keys_enum.addSlot("SDLK_GRAVE", SDLK_GRAVE); /**< '`' */
    keys_enum.addSlot("SDLK_A", SDLK_A); /**< 'a' */
    keys_enum.addSlot("SDLK_B", SDLK_B); /**< 'b' */
    keys_enum.addSlot("SDLK_C", SDLK_C); /**< 'c' */
    keys_enum.addSlot("SDLK_D", SDLK_D); /**< 'd' */
    keys_enum.addSlot("SDLK_E", SDLK_E); /**< 'e' */
    keys_enum.addSlot("SDLK_F", SDLK_F); /**< 'f' */
    keys_enum.addSlot("SDLK_G", SDLK_G); /**< 'g' */
    keys_enum.addSlot("SDLK_H", SDLK_H); /**< 'h' */
    keys_enum.addSlot("SDLK_I", SDLK_I); /**< 'i' */
    keys_enum.addSlot("SDLK_J", SDLK_J); /**< 'j' */
    keys_enum.addSlot("SDLK_K", SDLK_K); /**< 'k' */
    keys_enum.addSlot("SDLK_L", SDLK_L); /**< 'l' */
    keys_enum.addSlot("SDLK_M", SDLK_M); /**< 'm' */
    keys_enum.addSlot("SDLK_N", SDLK_N); /**< 'n' */
    keys_enum.addSlot("SDLK_O", SDLK_O); /**< 'o' */
    keys_enum.addSlot("SDLK_P", SDLK_P); /**< 'p' */
    keys_enum.addSlot("SDLK_Q", SDLK_Q); /**< 'q' */
    keys_enum.addSlot("SDLK_R", SDLK_R); /**< 'r' */
    keys_enum.addSlot("SDLK_S", SDLK_S); /**< 's' */
    keys_enum.addSlot("SDLK_T", SDLK_T); /**< 't' */
    keys_enum.addSlot("SDLK_U", SDLK_U); /**< 'u' */
    keys_enum.addSlot("SDLK_V", SDLK_V); /**< 'v' */
    keys_enum.addSlot("SDLK_W", SDLK_W); /**< 'w' */
    keys_enum.addSlot("SDLK_X", SDLK_X); /**< 'x' */
    keys_enum.addSlot("SDLK_Y", SDLK_Y); /**< 'y' */
    keys_enum.addSlot("SDLK_Z", SDLK_Z); /**< 'z' */
    keys_enum.addSlot("SDLK_LEFTBRACE", SDLK_LEFTBRACE); /**< '{' */
    keys_enum.addSlot("SDLK_PIPE", SDLK_PIPE); /**< '|' */
    keys_enum.addSlot("SDLK_RIGHTBRACE", SDLK_RIGHTBRACE); /**< '}' */
    keys_enum.addSlot("SDLK_TILDE", SDLK_TILDE); /**< '~' */
    keys_enum.addSlot("SDLK_DELETE", SDLK_DELETE); /**< '\x7F' */
    keys_enum.addSlot("SDLK_PLUSMINUS", SDLK_PLUSMINUS); /**< '\xB1' */
    keys_enum.addSlot("SDLK_CAPSLOCK", SDLK_CAPSLOCK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK) */
    keys_enum.addSlot("SDLK_F1", SDLK_F1); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1) */
    keys_enum.addSlot("SDLK_F2", SDLK_F2); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2) */
    keys_enum.addSlot("SDLK_F3", SDLK_F3); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3) */
    keys_enum.addSlot("SDLK_F4", SDLK_F4); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4) */
    keys_enum.addSlot("SDLK_F5", SDLK_F5); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5) */
    keys_enum.addSlot("SDLK_F6", SDLK_F6); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6) */
    keys_enum.addSlot("SDLK_F7", SDLK_F7); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7) */
    keys_enum.addSlot("SDLK_F8", SDLK_F8); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8) */
    keys_enum.addSlot("SDLK_F9", SDLK_F9); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9) */
    keys_enum.addSlot("SDLK_F10", SDLK_F10); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10) */
    keys_enum.addSlot("SDLK_F11", SDLK_F11); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11) */
    keys_enum.addSlot("SDLK_F12", SDLK_F12); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12) */
    keys_enum.addSlot("SDLK_PRINTSCREEN", SDLK_PRINTSCREEN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN) */
    keys_enum.addSlot("SDLK_SCROLLLOCK", SDLK_SCROLLLOCK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK) */
    keys_enum.addSlot("SDLK_PAUSE", SDLK_PAUSE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE) */
    keys_enum.addSlot("SDLK_INSERT", SDLK_INSERT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT) */
    keys_enum.addSlot("SDLK_HOME", SDLK_HOME); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME) */
    keys_enum.addSlot("SDLK_PAGEUP", SDLK_PAGEUP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP) */
    keys_enum.addSlot("SDLK_END", SDLK_END); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END) */
    keys_enum.addSlot("SDLK_PAGEDOWN", SDLK_PAGEDOWN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN) */
    keys_enum.addSlot("SDLK_RIGHT", SDLK_RIGHT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT) */
    keys_enum.addSlot("SDLK_LEFT", SDLK_LEFT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT) */
    keys_enum.addSlot("SDLK_DOWN", SDLK_DOWN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN) */
    keys_enum.addSlot("SDLK_UP", SDLK_UP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP) */
    keys_enum.addSlot("SDLK_NUMLOCKCLEAR", SDLK_NUMLOCKCLEAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR) */
    keys_enum.addSlot("SDLK_KP_DIVIDE", SDLK_KP_DIVIDE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE) */
    keys_enum.addSlot("SDLK_KP_MULTIPLY", SDLK_KP_MULTIPLY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY) */
    keys_enum.addSlot("SDLK_KP_MINUS", SDLK_KP_MINUS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS) */
    keys_enum.addSlot("SDLK_KP_PLUS", SDLK_KP_PLUS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS) */
    keys_enum.addSlot("SDLK_KP_ENTER", SDLK_KP_ENTER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER) */
    keys_enum.addSlot("SDLK_KP_1", SDLK_KP_1); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1) */
    keys_enum.addSlot("SDLK_KP_2", SDLK_KP_2); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2) */
    keys_enum.addSlot("SDLK_KP_3", SDLK_KP_3); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3) */
    keys_enum.addSlot("SDLK_KP_4", SDLK_KP_4); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4) */
    keys_enum.addSlot("SDLK_KP_5", SDLK_KP_5); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5) */
    keys_enum.addSlot("SDLK_KP_6", SDLK_KP_6); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6) */
    keys_enum.addSlot("SDLK_KP_7", SDLK_KP_7); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7) */
    keys_enum.addSlot("SDLK_KP_8", SDLK_KP_8); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8) */
    keys_enum.addSlot("SDLK_KP_9", SDLK_KP_9); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9) */
    keys_enum.addSlot("SDLK_KP_0", SDLK_KP_0); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0) */
    keys_enum.addSlot("SDLK_KP_PERIOD", SDLK_KP_PERIOD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD) */
    keys_enum.addSlot("SDLK_APPLICATION", SDLK_APPLICATION); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION) */
    keys_enum.addSlot("SDLK_POWER", SDLK_POWER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER) */
    keys_enum.addSlot("SDLK_KP_EQUALS", SDLK_KP_EQUALS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS) */
    keys_enum.addSlot("SDLK_F13", SDLK_F13); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13) */
    keys_enum.addSlot("SDLK_F14", SDLK_F14); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14) */
    keys_enum.addSlot("SDLK_F15", SDLK_F15); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15) */
    keys_enum.addSlot("SDLK_F16", SDLK_F16); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16) */
    keys_enum.addSlot("SDLK_F17", SDLK_F17); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17) */
    keys_enum.addSlot("SDLK_F18", SDLK_F18); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18) */
    keys_enum.addSlot("SDLK_F19", SDLK_F19); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19) */
    keys_enum.addSlot("SDLK_F20", SDLK_F20); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20) */
    keys_enum.addSlot("SDLK_F21", SDLK_F21); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21) */
    keys_enum.addSlot("SDLK_F22", SDLK_F22); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22) */
    keys_enum.addSlot("SDLK_F23", SDLK_F23); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23) */
    keys_enum.addSlot("SDLK_F24", SDLK_F24); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24) */
    keys_enum.addSlot("SDLK_EXECUTE", SDLK_EXECUTE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE) */
    keys_enum.addSlot("SDLK_HELP", SDLK_HELP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP) */
    keys_enum.addSlot("SDLK_MENU", SDLK_MENU); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU) */
    keys_enum.addSlot("SDLK_SELECT", SDLK_SELECT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT) */
    keys_enum.addSlot("SDLK_STOP", SDLK_STOP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP) */
    keys_enum.addSlot("SDLK_AGAIN", SDLK_AGAIN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN) */
    keys_enum.addSlot("SDLK_UNDO", SDLK_UNDO); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO) */
    keys_enum.addSlot("SDLK_CUT", SDLK_CUT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT) */
    keys_enum.addSlot("SDLK_COPY", SDLK_COPY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY) */
    keys_enum.addSlot("SDLK_PASTE", SDLK_PASTE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE) */
    keys_enum.addSlot("SDLK_FIND", SDLK_FIND); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND) */
    keys_enum.addSlot("SDLK_MUTE", SDLK_MUTE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE) */
    keys_enum.addSlot("SDLK_VOLUMEUP", SDLK_VOLUMEUP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP) */
    keys_enum.addSlot("SDLK_VOLUMEDOWN", SDLK_VOLUMEDOWN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN) */
    keys_enum.addSlot("SDLK_KP_COMMA", SDLK_KP_COMMA); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA) */
    keys_enum.addSlot("SDLK_KP_EQUALSAS400", SDLK_KP_EQUALSAS400); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400) */
    keys_enum.addSlot("SDLK_ALTERASE", SDLK_ALTERASE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE) */
    keys_enum.addSlot("SDLK_SYSREQ", SDLK_SYSREQ); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ) */
    keys_enum.addSlot("SDLK_CANCEL", SDLK_CANCEL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL) */
    keys_enum.addSlot("SDLK_CLEAR", SDLK_CLEAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR) */
    keys_enum.addSlot("SDLK_PRIOR", SDLK_PRIOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR) */
    keys_enum.addSlot("SDLK_RETURN2", SDLK_RETURN2); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2) */
    keys_enum.addSlot("SDLK_SEPARATOR", SDLK_SEPARATOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR) */
    keys_enum.addSlot("SDLK_OUT", SDLK_OUT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT) */
    keys_enum.addSlot("SDLK_OPER", SDLK_OPER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER) */
    keys_enum.addSlot("SDLK_CLEARAGAIN", SDLK_CLEARAGAIN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN) */
    keys_enum.addSlot("SDLK_CRSEL", SDLK_CRSEL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL) */
    keys_enum.addSlot("SDLK_EXSEL", SDLK_EXSEL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL) */
    keys_enum.addSlot("SDLK_KP_00", SDLK_KP_00); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00) */
    keys_enum.addSlot("SDLK_KP_000", SDLK_KP_000); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000) */
    keys_enum.addSlot("SDLK_THOUSANDSSEPARATOR", SDLK_THOUSANDSSEPARATOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR) */
    keys_enum.addSlot("SDLK_DECIMALSEPARATOR", SDLK_DECIMALSEPARATOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR) */
    keys_enum.addSlot("SDLK_CURRENCYUNIT", SDLK_CURRENCYUNIT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT) */
    keys_enum.addSlot("SDLK_CURRENCYSUBUNIT", SDLK_CURRENCYSUBUNIT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT) */
    keys_enum.addSlot("SDLK_KP_LEFTPAREN", SDLK_KP_LEFTPAREN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN) */
    keys_enum.addSlot("SDLK_KP_RIGHTPAREN", SDLK_KP_RIGHTPAREN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN) */
    keys_enum.addSlot("SDLK_KP_LEFTBRACE", SDLK_KP_LEFTBRACE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE) */
    keys_enum.addSlot("SDLK_KP_RIGHTBRACE", SDLK_KP_RIGHTBRACE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE) */
    keys_enum.addSlot("SDLK_KP_TAB", SDLK_KP_TAB); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB) */
    keys_enum.addSlot("SDLK_KP_BACKSPACE", SDLK_KP_BACKSPACE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE) */
    keys_enum.addSlot("SDLK_KP_A", SDLK_KP_A); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A) */
    keys_enum.addSlot("SDLK_KP_B", SDLK_KP_B); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B) */
    keys_enum.addSlot("SDLK_KP_C", SDLK_KP_C); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C) */
    keys_enum.addSlot("SDLK_KP_D", SDLK_KP_D); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D) */
    keys_enum.addSlot("SDLK_KP_E", SDLK_KP_E); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E) */
    keys_enum.addSlot("SDLK_KP_F", SDLK_KP_F); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F) */
    keys_enum.addSlot("SDLK_KP_XOR", SDLK_KP_XOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR) */
    keys_enum.addSlot("SDLK_KP_POWER", SDLK_KP_POWER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER) */
    keys_enum.addSlot("SDLK_KP_PERCENT", SDLK_KP_PERCENT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT) */
    keys_enum.addSlot("SDLK_KP_LESS", SDLK_KP_LESS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS) */
    keys_enum.addSlot("SDLK_KP_GREATER", SDLK_KP_GREATER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER) */
    keys_enum.addSlot("SDLK_KP_AMPERSAND", SDLK_KP_AMPERSAND); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND) */
    keys_enum.addSlot("SDLK_KP_DBLAMPERSAND", SDLK_KP_DBLAMPERSAND); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND) */
    keys_enum.addSlot("SDLK_KP_VERTICALBAR", SDLK_KP_VERTICALBAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR) */
    keys_enum.addSlot("SDLK_KP_DBLVERTICALBAR", SDLK_KP_DBLVERTICALBAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR) */
    keys_enum.addSlot("SDLK_KP_COLON", SDLK_KP_COLON); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON) */
    keys_enum.addSlot("SDLK_KP_HASH", SDLK_KP_HASH); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH) */
    keys_enum.addSlot("SDLK_KP_SPACE", SDLK_KP_SPACE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE) */
    keys_enum.addSlot("SDLK_KP_AT", SDLK_KP_AT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT) */
    keys_enum.addSlot("SDLK_KP_EXCLAM", SDLK_KP_EXCLAM); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM) */
    keys_enum.addSlot("SDLK_KP_MEMSTORE", SDLK_KP_MEMSTORE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE) */
    keys_enum.addSlot("SDLK_KP_MEMRECALL", SDLK_KP_MEMRECALL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL) */
    keys_enum.addSlot("SDLK_KP_MEMCLEAR", SDLK_KP_MEMCLEAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR) */
    keys_enum.addSlot("SDLK_KP_MEMADD", SDLK_KP_MEMADD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD) */
    keys_enum.addSlot("SDLK_KP_MEMSUBTRACT", SDLK_KP_MEMSUBTRACT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT) */
    keys_enum.addSlot("SDLK_KP_MEMMULTIPLY", SDLK_KP_MEMMULTIPLY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY) */
    keys_enum.addSlot("SDLK_KP_MEMDIVIDE", SDLK_KP_MEMDIVIDE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE) */
    keys_enum.addSlot("SDLK_KP_PLUSMINUS", SDLK_KP_PLUSMINUS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS) */
    keys_enum.addSlot("SDLK_KP_CLEAR", SDLK_KP_CLEAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR) */
    keys_enum.addSlot("SDLK_KP_CLEARENTRY", SDLK_KP_CLEARENTRY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY) */
    keys_enum.addSlot("SDLK_KP_BINARY", SDLK_KP_BINARY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY) */
    keys_enum.addSlot("SDLK_KP_OCTAL", SDLK_KP_OCTAL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL) */
    keys_enum.addSlot("SDLK_KP_DECIMAL", SDLK_KP_DECIMAL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL) */
    keys_enum.addSlot("SDLK_KP_HEXADECIMAL", SDLK_KP_HEXADECIMAL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL) */
    keys_enum.addSlot("SDLK_LCTRL", SDLK_LCTRL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL) */
    keys_enum.addSlot("SDLK_LSHIFT", SDLK_LSHIFT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT) */
    keys_enum.addSlot("SDLK_LALT", SDLK_LALT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT) */
    keys_enum.addSlot("SDLK_LGUI", SDLK_LGUI); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI) */
    keys_enum.addSlot("SDLK_RCTRL", SDLK_RCTRL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL) */
    keys_enum.addSlot("SDLK_RSHIFT", SDLK_RSHIFT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT) */
    keys_enum.addSlot("SDLK_RALT", SDLK_RALT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT) */
    keys_enum.addSlot("SDLK_RGUI", SDLK_RGUI); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI) */
    keys_enum.addSlot("SDLK_MODE", SDLK_MODE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE) */
    keys_enum.addSlot("SDLK_SLEEP", SDLK_SLEEP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP) */
    keys_enum.addSlot("SDLK_WAKE", SDLK_WAKE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WAKE) */
    keys_enum.addSlot("SDLK_CHANNEL_INCREMENT", SDLK_CHANNEL_INCREMENT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CHANNEL_INCREMENT) */
    keys_enum.addSlot("SDLK_CHANNEL_DECREMENT", SDLK_CHANNEL_DECREMENT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CHANNEL_DECREMENT) */
    keys_enum.addSlot("SDLK_MEDIA_PLAY", SDLK_MEDIA_PLAY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PLAY) */
    keys_enum.addSlot("SDLK_MEDIA_PAUSE", SDLK_MEDIA_PAUSE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PAUSE) */
    keys_enum.addSlot("SDLK_MEDIA_RECORD", SDLK_MEDIA_RECORD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_RECORD) */
    keys_enum.addSlot("SDLK_MEDIA_FAST_FORWARD", SDLK_MEDIA_FAST_FORWARD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_FAST_FORWARD) */
    keys_enum.addSlot("SDLK_MEDIA_REWIND", SDLK_MEDIA_REWIND); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_REWIND) */
    keys_enum.addSlot("SDLK_MEDIA_NEXT_TRACK", SDLK_MEDIA_NEXT_TRACK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_NEXT_TRACK) */
    keys_enum.addSlot("SDLK_MEDIA_PREVIOUS_TRACK", SDLK_MEDIA_PREVIOUS_TRACK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PREVIOUS_TRACK) */
    keys_enum.addSlot("SDLK_MEDIA_STOP", SDLK_MEDIA_STOP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_STOP) */
    keys_enum.addSlot("SDLK_MEDIA_EJECT", SDLK_MEDIA_EJECT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_EJECT) */
    keys_enum.addSlot("SDLK_MEDIA_PLAY_PAUSE", SDLK_MEDIA_PLAY_PAUSE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PLAY_PAUSE) */
    keys_enum.addSlot("SDLK_MEDIA_SELECT", SDLK_MEDIA_SELECT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_SELECT) */
    keys_enum.addSlot("SDLK_AC_NEW", SDLK_AC_NEW); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_NEW) */
    keys_enum.addSlot("SDLK_AC_OPEN", SDLK_AC_OPEN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_OPEN) */
    keys_enum.addSlot("SDLK_AC_CLOSE", SDLK_AC_CLOSE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_CLOSE) */
    keys_enum.addSlot("SDLK_AC_EXIT", SDLK_AC_EXIT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_EXIT) */
    keys_enum.addSlot("SDLK_AC_SAVE", SDLK_AC_SAVE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SAVE) */
    keys_enum.addSlot("SDLK_AC_PRINT", SDLK_AC_PRINT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_PRINT) */
    keys_enum.addSlot("SDLK_AC_PROPERTIES", SDLK_AC_PROPERTIES); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_PROPERTIES) */
    keys_enum.addSlot("SDLK_AC_SEARCH", SDLK_AC_SEARCH); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH) */
    keys_enum.addSlot("SDLK_AC_HOME", SDLK_AC_HOME); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME) */
    keys_enum.addSlot("SDLK_AC_BACK", SDLK_AC_BACK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK) */
    keys_enum.addSlot("SDLK_AC_FORWARD", SDLK_AC_FORWARD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD) */
    keys_enum.addSlot("SDLK_AC_STOP", SDLK_AC_STOP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP) */
    keys_enum.addSlot("SDLK_AC_REFRESH", SDLK_AC_REFRESH); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH) */
    keys_enum.addSlot("SDLK_AC_BOOKMARKS", SDLK_AC_BOOKMARKS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS) */
    keys_enum.addSlot("SDLK_SOFTLEFT", SDLK_SOFTLEFT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTLEFT) */
    keys_enum.addSlot("SDLK_SOFTRIGHT", SDLK_SOFTRIGHT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTRIGHT) */
    keys_enum.addSlot("SDLK_CALL", SDLK_CALL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALL) */
    keys_enum.addSlot("SDLK_ENDCALL", SDLK_ENDCALL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ENDCALL) */
    keys_enum.addSlot("SDLK_LEFT_TAB", SDLK_LEFT_TAB); /**< Extended key Left Tab */
    keys_enum.addSlot("SDLK_LEVEL5_SHIFT", SDLK_LEVEL5_SHIFT); /**< Extended key Level 5 Shift */
    keys_enum.addSlot("SDLK_MULTI_KEY_COMPOSE", SDLK_MULTI_KEY_COMPOSE); /**< Extended key Multi-key Compose */
    keys_enum.addSlot("SDLK_LMETA", SDLK_LMETA); /**< Extended key Left Meta */
    keys_enum.addSlot("SDLK_RMETA", SDLK_RMETA); /**< Extended key Right Meta */
    keys_enum.addSlot("SDLK_LHYPER", SDLK_LHYPER); /**< Extended key Left Hyper */
    keys_enum.addSlot("SDLK_RHYPER", SDLK_RHYPER); /**< Extended key Right Hyper */
  }

  if (!vm.hasEntry("Controls"))
  {
    auto controls_enum = vm.addEnum("Controls");
    controls_enum.addSlot("LEFT", (int)Control::LEFT);
    controls_enum.addSlot("RIGHT", (int)Control::RIGHT);
    controls_enum.addSlot("UP", (int)Control::UP);
    controls_enum.addSlot("DOWN", (int)Control::DOWN);

    controls_enum.addSlot("JUMP", (int)Control::JUMP);
    controls_enum.addSlot("ACTION", (int)Control::ACTION);
    controls_enum.addSlot("ITEM", (int)Control::ITEM);

    controls_enum.addSlot("START", (int)Control::START);
    controls_enum.addSlot("ESCAPE", (int)Control::ESCAPE);
    controls_enum.addSlot("MENU_SELECT", (int)Control::MENU_SELECT);
    controls_enum.addSlot("MENU_SELECT_SPACE", (int)Control::MENU_SELECT_SPACE);
    controls_enum.addSlot("MENU_BACK", (int)Control::MENU_BACK);
    controls_enum.addSlot("REMOVE", (int)Control::REMOVE);

    controls_enum.addSlot("CHEAT_MENU", (int)Control::CHEAT_MENU);
    controls_enum.addSlot("DEBUG_MENU", (int)Control::DEBUG_MENU);
    controls_enum.addSlot("CONSOLE", (int)Control::CONSOLE);

    controls_enum.addSlot("PEEK_LEFT", (int)Control::PEEK_LEFT);
    controls_enum.addSlot("PEEK_RIGHT", (int)Control::PEEK_RIGHT);
    controls_enum.addSlot("PEEK_UP", (int)Control::PEEK_UP);
    controls_enum.addSlot("PEEK_DOWN", (int)Control::PEEK_DOWN);
  }
}