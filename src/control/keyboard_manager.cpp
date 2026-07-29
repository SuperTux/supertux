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
          param_count = function->getNumOfParams().second;
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
            param_count = function->getNumOfParams().second;

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

    #define register_key(key) \
      keys_enum.addSlot(#key, key);

    register_key(SDLK_UNKNOWN); /**< 0 */
    register_key(SDLK_RETURN); /**< '\r' */
    register_key(SDLK_ESCAPE); /**< '\x1B' */
    register_key(SDLK_BACKSPACE); /**< '\b' */
    register_key(SDLK_TAB); /**< '\t' */
    register_key(SDLK_SPACE); /**< ' ' */
    register_key(SDLK_EXCLAIM); /**< '!' */
    register_key(SDLK_DBLAPOSTROPHE); /**< '"' */
    register_key(SDLK_HASH); /**< '#' */
    register_key(SDLK_DOLLAR); /**< '$' */
    register_key(SDLK_PERCENT); /**< '%' */
    register_key(SDLK_AMPERSAND); /**< '&' */
    register_key(SDLK_APOSTROPHE); /**< '\'' */
    register_key(SDLK_LEFTPAREN); /**< '(' */
    register_key(SDLK_RIGHTPAREN); /**< ')' */
    register_key(SDLK_ASTERISK); /**< '*' */
    register_key(SDLK_PLUS); /**< '+' */
    register_key(SDLK_COMMA); /**< ',' */
    register_key(SDLK_MINUS); /**< '-' */
    register_key(SDLK_PERIOD); /**< '.' */
    register_key(SDLK_SLASH); /**< '/' */
    register_key(SDLK_0); /**< '0' */
    register_key(SDLK_1); /**< '1' */
    register_key(SDLK_2); /**< '2' */
    register_key(SDLK_3); /**< '3' */
    register_key(SDLK_4); /**< '4' */
    register_key(SDLK_5); /**< '5' */
    register_key(SDLK_6); /**< '6' */
    register_key(SDLK_7); /**< '7' */
    register_key(SDLK_8); /**< '8' */
    register_key(SDLK_9); /**< '9' */
    register_key(SDLK_COLON); /**< ':' */
    register_key(SDLK_SEMICOLON); /**< ';' */
    register_key(SDLK_LESS); /**< '<' */
    register_key(SDLK_EQUALS); /**< '=' */
    register_key(SDLK_GREATER); /**< '>' */
    register_key(SDLK_QUESTION); /**< '?' */
    register_key(SDLK_AT); /**< '@' */
    register_key(SDLK_LEFTBRACKET); /**< '[' */
    register_key(SDLK_BACKSLASH); /**< '\\' */
    register_key(SDLK_RIGHTBRACKET); /**< ']' */
    register_key(SDLK_CARET); /**< '^' */
    register_key(SDLK_UNDERSCORE); /**< '_' */
    register_key(SDLK_GRAVE); /**< '`' */
    register_key(SDLK_A); /**< 'a' */
    register_key(SDLK_B); /**< 'b' */
    register_key(SDLK_C); /**< 'c' */
    register_key(SDLK_D); /**< 'd' */
    register_key(SDLK_E); /**< 'e' */
    register_key(SDLK_F); /**< 'f' */
    register_key(SDLK_G); /**< 'g' */
    register_key(SDLK_H); /**< 'h' */
    register_key(SDLK_I); /**< 'i' */
    register_key(SDLK_J); /**< 'j' */
    register_key(SDLK_K); /**< 'k' */
    register_key(SDLK_L); /**< 'l' */
    register_key(SDLK_M); /**< 'm' */
    register_key(SDLK_N); /**< 'n' */
    register_key(SDLK_O); /**< 'o' */
    register_key(SDLK_P); /**< 'p' */
    register_key(SDLK_Q); /**< 'q' */
    register_key(SDLK_R); /**< 'r' */
    register_key(SDLK_S); /**< 's' */
    register_key(SDLK_T); /**< 't' */
    register_key(SDLK_U); /**< 'u' */
    register_key(SDLK_V); /**< 'v' */
    register_key(SDLK_W); /**< 'w' */
    register_key(SDLK_X); /**< 'x' */
    register_key(SDLK_Y); /**< 'y' */
    register_key(SDLK_Z); /**< 'z' */
    register_key(SDLK_LEFTBRACE); /**< '{' */
    register_key(SDLK_PIPE); /**< '|' */
    register_key(SDLK_RIGHTBRACE); /**< '}' */
    register_key(SDLK_TILDE); /**< '~' */
    register_key(SDLK_DELETE); /**< '\x7F' */
    register_key(SDLK_PLUSMINUS); /**< '\xB1' */
    register_key(SDLK_CAPSLOCK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK) */
    register_key(SDLK_F1); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1) */
    register_key(SDLK_F2); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2) */
    register_key(SDLK_F3); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3) */
    register_key(SDLK_F4); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4) */
    register_key(SDLK_F5); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5) */
    register_key(SDLK_F6); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6) */
    register_key(SDLK_F7); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7) */
    register_key(SDLK_F8); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8) */
    register_key(SDLK_F9); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9) */
    register_key(SDLK_F10); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10) */
    register_key(SDLK_F11); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11) */
    register_key(SDLK_F12); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12) */
    register_key(SDLK_PRINTSCREEN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN) */
    register_key(SDLK_SCROLLLOCK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK) */
    register_key(SDLK_PAUSE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE) */
    register_key(SDLK_INSERT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT) */
    register_key(SDLK_HOME); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME) */
    register_key(SDLK_PAGEUP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP) */
    register_key(SDLK_END); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END) */
    register_key(SDLK_PAGEDOWN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN) */
    register_key(SDLK_RIGHT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT) */
    register_key(SDLK_LEFT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT) */
    register_key(SDLK_DOWN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN) */
    register_key(SDLK_UP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP) */
    register_key(SDLK_NUMLOCKCLEAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR) */
    register_key(SDLK_KP_DIVIDE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE) */
    register_key(SDLK_KP_MULTIPLY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY) */
    register_key(SDLK_KP_MINUS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS) */
    register_key(SDLK_KP_PLUS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS) */
    register_key(SDLK_KP_ENTER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER) */
    register_key(SDLK_KP_1); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1) */
    register_key(SDLK_KP_2); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2) */
    register_key(SDLK_KP_3); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3) */
    register_key(SDLK_KP_4); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4) */
    register_key(SDLK_KP_5); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5) */
    register_key(SDLK_KP_6); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6) */
    register_key(SDLK_KP_7); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7) */
    register_key(SDLK_KP_8); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8) */
    register_key(SDLK_KP_9); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9) */
    register_key(SDLK_KP_0); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0) */
    register_key(SDLK_KP_PERIOD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD) */
    register_key(SDLK_APPLICATION); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION) */
    register_key(SDLK_POWER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER) */
    register_key(SDLK_KP_EQUALS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS) */
    register_key(SDLK_F13); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13) */
    register_key(SDLK_F14); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14) */
    register_key(SDLK_F15); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15) */
    register_key(SDLK_F16); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16) */
    register_key(SDLK_F17); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17) */
    register_key(SDLK_F18); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18) */
    register_key(SDLK_F19); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19) */
    register_key(SDLK_F20); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20) */
    register_key(SDLK_F21); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21) */
    register_key(SDLK_F22); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22) */
    register_key(SDLK_F23); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23) */
    register_key(SDLK_F24); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24) */
    register_key(SDLK_EXECUTE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE) */
    register_key(SDLK_HELP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP) */
    register_key(SDLK_MENU); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU) */
    register_key(SDLK_SELECT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT) */
    register_key(SDLK_STOP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP) */
    register_key(SDLK_AGAIN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN) */
    register_key(SDLK_UNDO); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO) */
    register_key(SDLK_CUT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT) */
    register_key(SDLK_COPY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY) */
    register_key(SDLK_PASTE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE) */
    register_key(SDLK_FIND); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND) */
    register_key(SDLK_MUTE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE) */
    register_key(SDLK_VOLUMEUP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP) */
    register_key(SDLK_VOLUMEDOWN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN) */
    register_key(SDLK_KP_COMMA); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA) */
    register_key(SDLK_KP_EQUALSAS400); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400) */
    register_key(SDLK_ALTERASE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE) */
    register_key(SDLK_SYSREQ); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ) */
    register_key(SDLK_CANCEL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL) */
    register_key(SDLK_CLEAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR) */
    register_key(SDLK_PRIOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR) */
    register_key(SDLK_RETURN2); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2) */
    register_key(SDLK_SEPARATOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR) */
    register_key(SDLK_OUT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT) */
    register_key(SDLK_OPER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER) */
    register_key(SDLK_CLEARAGAIN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN) */
    register_key(SDLK_CRSEL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL) */
    register_key(SDLK_EXSEL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL) */
    register_key(SDLK_KP_00); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00) */
    register_key(SDLK_KP_000); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000) */
    register_key(SDLK_THOUSANDSSEPARATOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR) */
    register_key(SDLK_DECIMALSEPARATOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR) */
    register_key(SDLK_CURRENCYUNIT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT) */
    register_key(SDLK_CURRENCYSUBUNIT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT) */
    register_key(SDLK_KP_LEFTPAREN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN) */
    register_key(SDLK_KP_RIGHTPAREN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN) */
    register_key(SDLK_KP_LEFTBRACE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE) */
    register_key(SDLK_KP_RIGHTBRACE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE) */
    register_key(SDLK_KP_TAB); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB) */
    register_key(SDLK_KP_BACKSPACE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE) */
    register_key(SDLK_KP_A); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A) */
    register_key(SDLK_KP_B); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B) */
    register_key(SDLK_KP_C); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C) */
    register_key(SDLK_KP_D); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D) */
    register_key(SDLK_KP_E); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E) */
    register_key(SDLK_KP_F); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F) */
    register_key(SDLK_KP_XOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR) */
    register_key(SDLK_KP_POWER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER) */
    register_key(SDLK_KP_PERCENT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT) */
    register_key(SDLK_KP_LESS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS) */
    register_key(SDLK_KP_GREATER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER) */
    register_key(SDLK_KP_AMPERSAND); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND) */
    register_key(SDLK_KP_DBLAMPERSAND); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND) */
    register_key(SDLK_KP_VERTICALBAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR) */
    register_key(SDLK_KP_DBLVERTICALBAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR) */
    register_key(SDLK_KP_COLON); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON) */
    register_key(SDLK_KP_HASH); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH) */
    register_key(SDLK_KP_SPACE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE) */
    register_key(SDLK_KP_AT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT) */
    register_key(SDLK_KP_EXCLAM); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM) */
    register_key(SDLK_KP_MEMSTORE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE) */
    register_key(SDLK_KP_MEMRECALL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL) */
    register_key(SDLK_KP_MEMCLEAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR) */
    register_key(SDLK_KP_MEMADD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD) */
    register_key(SDLK_KP_MEMSUBTRACT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT) */
    register_key(SDLK_KP_MEMMULTIPLY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY) */
    register_key(SDLK_KP_MEMDIVIDE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE) */
    register_key(SDLK_KP_PLUSMINUS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS) */
    register_key(SDLK_KP_CLEAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR) */
    register_key(SDLK_KP_CLEARENTRY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY) */
    register_key(SDLK_KP_BINARY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY) */
    register_key(SDLK_KP_OCTAL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL) */
    register_key(SDLK_KP_DECIMAL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL) */
    register_key(SDLK_KP_HEXADECIMAL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL) */
    register_key(SDLK_LCTRL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL) */
    register_key(SDLK_LSHIFT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT) */
    register_key(SDLK_LALT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT) */
    register_key(SDLK_LGUI); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI) */
    register_key(SDLK_RCTRL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL) */
    register_key(SDLK_RSHIFT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT) */
    register_key(SDLK_RALT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT) */
    register_key(SDLK_RGUI); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI) */
    register_key(SDLK_MODE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE) */
    register_key(SDLK_SLEEP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP) */
    register_key(SDLK_WAKE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WAKE) */
    register_key(SDLK_CHANNEL_INCREMENT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CHANNEL_INCREMENT) */
    register_key(SDLK_CHANNEL_DECREMENT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CHANNEL_DECREMENT) */
    register_key(SDLK_MEDIA_PLAY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PLAY) */
    register_key(SDLK_MEDIA_PAUSE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PAUSE) */
    register_key(SDLK_MEDIA_RECORD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_RECORD) */
    register_key(SDLK_MEDIA_FAST_FORWARD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_FAST_FORWARD) */
    register_key(SDLK_MEDIA_REWIND); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_REWIND) */
    register_key(SDLK_MEDIA_NEXT_TRACK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_NEXT_TRACK) */
    register_key(SDLK_MEDIA_PREVIOUS_TRACK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PREVIOUS_TRACK) */
    register_key(SDLK_MEDIA_STOP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_STOP) */
    register_key(SDLK_MEDIA_EJECT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_EJECT) */
    register_key(SDLK_MEDIA_PLAY_PAUSE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PLAY_PAUSE) */
    register_key(SDLK_MEDIA_SELECT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_SELECT) */
    register_key(SDLK_AC_NEW); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_NEW) */
    register_key(SDLK_AC_OPEN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_OPEN) */
    register_key(SDLK_AC_CLOSE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_CLOSE) */
    register_key(SDLK_AC_EXIT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_EXIT) */
    register_key(SDLK_AC_SAVE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SAVE) */
    register_key(SDLK_AC_PRINT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_PRINT) */
    register_key(SDLK_AC_PROPERTIES); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_PROPERTIES) */
    register_key(SDLK_AC_SEARCH); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH) */
    register_key(SDLK_AC_HOME); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME) */
    register_key(SDLK_AC_BACK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK) */
    register_key(SDLK_AC_FORWARD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD) */
    register_key(SDLK_AC_STOP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP) */
    register_key(SDLK_AC_REFRESH); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH) */
    register_key(SDLK_AC_BOOKMARKS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS) */
    register_key(SDLK_SOFTLEFT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTLEFT) */
    register_key(SDLK_SOFTRIGHT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTRIGHT) */
    register_key(SDLK_CALL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALL) */
    register_key(SDLK_ENDCALL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ENDCALL) */
    register_key(SDLK_LEFT_TAB); /**< Extended key Left Tab */
    register_key(SDLK_LEVEL5_SHIFT); /**< Extended key Level 5 Shift */
    register_key(SDLK_MULTI_KEY_COMPOSE); /**< Extended key Multi-key Compose */
    register_key(SDLK_LMETA); /**< Extended key Left Meta */
    register_key(SDLK_RMETA); /**< Extended key Right Meta */
    register_key(SDLK_LHYPER); /**< Extended key Left Hyper */
    register_key(SDLK_RHYPER); /**< Extended key Right Hyper */
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