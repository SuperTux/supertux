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
  bool console_key_pressed =
    key_mapping != m_keyboard_config.m_keymap.end() &&
    key_mapping->second.control == Control::CONSOLE;

  if (console_key_pressed || Console::current()->hasFocus())
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
    std::optional<KeyboardConfig::PlayerControl> control;
    if (key_mapping == m_keyboard_config.m_keymap.end())
    {
      // default action: update controls
      //log_debug << "Key " << event.key.SDL_Keycode.sym << " is unbound" << std::endl;
    }
    else
    {
      control = key_mapping->second;
      bool value = (event.type == SDL_EVENT_KEY_DOWN);

      if (control->player >= m_parent->get_num_users())
        return;

      m_parent->get_controller(control->player).set_control(control->control, value);

      if (m_keyboard_config.m_jump_with_up_kbd && control->control == Control::UP) {
        m_parent->get_controller(control->player).set_jump_key_with_up(value);
      }
    }

    handle_squirrel_keyboard_callback(event, control);
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
  else if (!MenuManager::instance().is_active())
  {
    handle_squirrel_text_input_callback(event);
  }
}

void
KeyboardManager::process_console_key_event(const SDL_KeyboardEvent& event)
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
    return;
  }

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

    // This function definition uses KEY_{key_name} on the Squirrel side
    // and SDLK_{key_name} on the SDL side.
    // Example: SDLK_RETURN gets registered as KEY_RETURN.
    #define register_key(key) \
      keys_enum.addSlot("KEY_"#key, SDLK_ ## key);

    register_key(UNKNOWN); /**< 0 */
    register_key(RETURN); /**< '\r' */
    register_key(ESCAPE); /**< '\x1B' */
    register_key(BACKSPACE); /**< '\b' */
    register_key(TAB); /**< '\t' */
    register_key(SPACE); /**< ' ' */
    register_key(EXCLAIM); /**< '!' */
    register_key(DBLAPOSTROPHE); /**< '"' */
    register_key(HASH); /**< '#' */
    register_key(DOLLAR); /**< '$' */
    register_key(PERCENT); /**< '%' */
    register_key(AMPERSAND); /**< '&' */
    register_key(APOSTROPHE); /**< '\'' */
    register_key(LEFTPAREN); /**< '(' */
    register_key(RIGHTPAREN); /**< ')' */
    register_key(ASTERISK); /**< '*' */
    register_key(PLUS); /**< '+' */
    register_key(COMMA); /**< ',' */
    register_key(MINUS); /**< '-' */
    register_key(PERIOD); /**< '.' */
    register_key(SLASH); /**< '/' */
    register_key(0); /**< '0' */
    register_key(1); /**< '1' */
    register_key(2); /**< '2' */
    register_key(3); /**< '3' */
    register_key(4); /**< '4' */
    register_key(5); /**< '5' */
    register_key(6); /**< '6' */
    register_key(7); /**< '7' */
    register_key(8); /**< '8' */
    register_key(9); /**< '9' */
    register_key(COLON); /**< ':' */
    register_key(SEMICOLON); /**< ';' */
    register_key(LESS); /**< '<' */
    register_key(EQUALS); /**< '=' */
    register_key(GREATER); /**< '>' */
    register_key(QUESTION); /**< '?' */
    register_key(AT); /**< '@' */
    register_key(LEFTBRACKET); /**< '[' */
    register_key(BACKSLASH); /**< '\\' */
    register_key(RIGHTBRACKET); /**< ']' */
    register_key(CARET); /**< '^' */
    register_key(UNDERSCORE); /**< '_' */
    register_key(GRAVE); /**< '`' */
    register_key(A); /**< 'a' */
    register_key(B); /**< 'b' */
    register_key(C); /**< 'c' */
    register_key(D); /**< 'd' */
    register_key(E); /**< 'e' */
    register_key(F); /**< 'f' */
    register_key(G); /**< 'g' */
    register_key(H); /**< 'h' */
    register_key(I); /**< 'i' */
    register_key(J); /**< 'j' */
    register_key(K); /**< 'k' */
    register_key(L); /**< 'l' */
    register_key(M); /**< 'm' */
    register_key(N); /**< 'n' */
    register_key(O); /**< 'o' */
    register_key(P); /**< 'p' */
    register_key(Q); /**< 'q' */
    register_key(R); /**< 'r' */
    register_key(S); /**< 's' */
    register_key(T); /**< 't' */
    register_key(U); /**< 'u' */
    register_key(V); /**< 'v' */
    register_key(W); /**< 'w' */
    register_key(X); /**< 'x' */
    register_key(Y); /**< 'y' */
    register_key(Z); /**< 'z' */
    register_key(LEFTBRACE); /**< '{' */
    register_key(PIPE); /**< '|' */
    register_key(RIGHTBRACE); /**< '}' */
    register_key(TILDE); /**< '~' */
    register_key(DELETE); /**< '\x7F' */
    register_key(PLUSMINUS); /**< '\xB1' */
    register_key(CAPSLOCK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK) */
    register_key(F1); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1) */
    register_key(F2); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2) */
    register_key(F3); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3) */
    register_key(F4); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4) */
    register_key(F5); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5) */
    register_key(F6); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6) */
    register_key(F7); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7) */
    register_key(F8); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8) */
    register_key(F9); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9) */
    register_key(F10); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10) */
    register_key(F11); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11) */
    register_key(F12); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12) */
    register_key(PRINTSCREEN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN) */
    register_key(SCROLLLOCK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK) */
    register_key(PAUSE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE) */
    register_key(INSERT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT) */
    register_key(HOME); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME) */
    register_key(PAGEUP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP) */
    register_key(END); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END) */
    register_key(PAGEDOWN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN) */
    register_key(RIGHT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT) */
    register_key(LEFT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT) */
    register_key(DOWN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN) */
    register_key(UP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP) */
    register_key(NUMLOCKCLEAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR) */
    register_key(KP_DIVIDE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE) */
    register_key(KP_MULTIPLY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY) */
    register_key(KP_MINUS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS) */
    register_key(KP_PLUS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS) */
    register_key(KP_ENTER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER) */
    register_key(KP_1); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1) */
    register_key(KP_2); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2) */
    register_key(KP_3); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3) */
    register_key(KP_4); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4) */
    register_key(KP_5); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5) */
    register_key(KP_6); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6) */
    register_key(KP_7); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7) */
    register_key(KP_8); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8) */
    register_key(KP_9); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9) */
    register_key(KP_0); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0) */
    register_key(KP_PERIOD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD) */
    register_key(APPLICATION); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION) */
    register_key(POWER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER) */
    register_key(KP_EQUALS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS) */
    register_key(F13); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13) */
    register_key(F14); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14) */
    register_key(F15); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15) */
    register_key(F16); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16) */
    register_key(F17); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17) */
    register_key(F18); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18) */
    register_key(F19); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19) */
    register_key(F20); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20) */
    register_key(F21); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21) */
    register_key(F22); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22) */
    register_key(F23); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23) */
    register_key(F24); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24) */
    register_key(EXECUTE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE) */
    register_key(HELP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP) */
    register_key(MENU); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU) */
    register_key(SELECT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT) */
    register_key(STOP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP) */
    register_key(AGAIN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN) */
    register_key(UNDO); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO) */
    register_key(CUT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT) */
    register_key(COPY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY) */
    register_key(PASTE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE) */
    register_key(FIND); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND) */
    register_key(MUTE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE) */
    register_key(VOLUMEUP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP) */
    register_key(VOLUMEDOWN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN) */
    register_key(KP_COMMA); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA) */
    register_key(KP_EQUALSAS400); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400) */
    register_key(ALTERASE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE) */
    register_key(SYSREQ); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ) */
    register_key(CANCEL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL) */
    register_key(CLEAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR) */
    register_key(PRIOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR) */
    register_key(RETURN2); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2) */
    register_key(SEPARATOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR) */
    register_key(OUT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT) */
    register_key(OPER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER) */
    register_key(CLEARAGAIN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN) */
    register_key(CRSEL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL) */
    register_key(EXSEL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL) */
    register_key(KP_00); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00) */
    register_key(KP_000); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000) */
    register_key(THOUSANDSSEPARATOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR) */
    register_key(DECIMALSEPARATOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR) */
    register_key(CURRENCYUNIT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT) */
    register_key(CURRENCYSUBUNIT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT) */
    register_key(KP_LEFTPAREN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN) */
    register_key(KP_RIGHTPAREN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN) */
    register_key(KP_LEFTBRACE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE) */
    register_key(KP_RIGHTBRACE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE) */
    register_key(KP_TAB); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB) */
    register_key(KP_BACKSPACE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE) */
    register_key(KP_A); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A) */
    register_key(KP_B); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B) */
    register_key(KP_C); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C) */
    register_key(KP_D); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D) */
    register_key(KP_E); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E) */
    register_key(KP_F); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F) */
    register_key(KP_XOR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR) */
    register_key(KP_POWER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER) */
    register_key(KP_PERCENT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT) */
    register_key(KP_LESS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS) */
    register_key(KP_GREATER); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER) */
    register_key(KP_AMPERSAND); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND) */
    register_key(KP_DBLAMPERSAND); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND) */
    register_key(KP_VERTICALBAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR) */
    register_key(KP_DBLVERTICALBAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR) */
    register_key(KP_COLON); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON) */
    register_key(KP_HASH); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH) */
    register_key(KP_SPACE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE) */
    register_key(KP_AT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT) */
    register_key(KP_EXCLAM); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM) */
    register_key(KP_MEMSTORE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE) */
    register_key(KP_MEMRECALL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL) */
    register_key(KP_MEMCLEAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR) */
    register_key(KP_MEMADD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD) */
    register_key(KP_MEMSUBTRACT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT) */
    register_key(KP_MEMMULTIPLY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY) */
    register_key(KP_MEMDIVIDE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE) */
    register_key(KP_PLUSMINUS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS) */
    register_key(KP_CLEAR); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR) */
    register_key(KP_CLEARENTRY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY) */
    register_key(KP_BINARY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY) */
    register_key(KP_OCTAL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL) */
    register_key(KP_DECIMAL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL) */
    register_key(KP_HEXADECIMAL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL) */
    register_key(LCTRL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL) */
    register_key(LSHIFT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT) */
    register_key(LALT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT) */
    register_key(LGUI); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI) */
    register_key(RCTRL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL) */
    register_key(RSHIFT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT) */
    register_key(RALT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT) */
    register_key(RGUI); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI) */
    register_key(MODE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE) */
    register_key(SLEEP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP) */
    register_key(WAKE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WAKE) */
    register_key(CHANNEL_INCREMENT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CHANNEL_INCREMENT) */
    register_key(CHANNEL_DECREMENT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CHANNEL_DECREMENT) */
    register_key(MEDIA_PLAY); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PLAY) */
    register_key(MEDIA_PAUSE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PAUSE) */
    register_key(MEDIA_RECORD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_RECORD) */
    register_key(MEDIA_FAST_FORWARD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_FAST_FORWARD) */
    register_key(MEDIA_REWIND); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_REWIND) */
    register_key(MEDIA_NEXT_TRACK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_NEXT_TRACK) */
    register_key(MEDIA_PREVIOUS_TRACK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PREVIOUS_TRACK) */
    register_key(MEDIA_STOP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_STOP) */
    register_key(MEDIA_EJECT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_EJECT) */
    register_key(MEDIA_PLAY_PAUSE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PLAY_PAUSE) */
    register_key(MEDIA_SELECT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_SELECT) */
    register_key(AC_NEW); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_NEW) */
    register_key(AC_OPEN); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_OPEN) */
    register_key(AC_CLOSE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_CLOSE) */
    register_key(AC_EXIT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_EXIT) */
    register_key(AC_SAVE); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SAVE) */
    register_key(AC_PRINT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_PRINT) */
    register_key(AC_PROPERTIES); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_PROPERTIES) */
    register_key(AC_SEARCH); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH) */
    register_key(AC_HOME); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME) */
    register_key(AC_BACK); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK) */
    register_key(AC_FORWARD); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD) */
    register_key(AC_STOP); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP) */
    register_key(AC_REFRESH); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH) */
    register_key(AC_BOOKMARKS); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS) */
    register_key(SOFTLEFT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTLEFT) */
    register_key(SOFTRIGHT); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTRIGHT) */
    register_key(CALL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALL) */
    register_key(ENDCALL); /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ENDCALL) */
    register_key(LEFT_TAB); /**< Extended key Left Tab */
    register_key(LEVEL5_SHIFT); /**< Extended key Level 5 Shift */
    register_key(MULTI_KEY_COMPOSE); /**< Extended key Multi-key Compose */
    register_key(LMETA); /**< Extended key Left Meta */
    register_key(RMETA); /**< Extended key Right Meta */
    register_key(LHYPER); /**< Extended key Left Hyper */
    register_key(RHYPER); /**< Extended key Right Hyper */
  }

  if (!vm.hasEntry("Controls"))
  {
    auto controls_enum = vm.addEnum("Controls");
    
    #define register_control(control_key) \
      controls_enum.addSlot(#control_key, (int)Control::control_key);

    register_control(LEFT);
    register_control(RIGHT);
    register_control(UP);
    register_control(DOWN);

    register_control(JUMP);
    register_control(ACTION);
    register_control(ITEM);

    register_control(START);
    register_control(ESCAPE);
    register_control(MENU_SELECT);
    register_control(MENU_SELECT_SPACE);
    register_control(MENU_BACK);
    register_control(REMOVE);

    register_control(CHEAT_MENU);
    register_control(DEBUG_MENU);
    register_control(CONSOLE);

    register_control(PEEK_LEFT);
    register_control(PEEK_RIGHT);
    register_control(PEEK_UP);
    register_control(PEEK_DOWN);
  }
}

void
KeyboardManager::handle_squirrel_keyboard_callback(const SDL_KeyboardEvent& event, std::optional<KeyboardConfig::PlayerControl> control)
{
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

      if (control)
      {
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
            ssq_vm.callFunc(*function, ssq_vm, (int)control->control, (int)control->player);
          }
          else
          {
            ssq_vm.callFunc(*function, ssq_vm, (int)control->control);
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

void
KeyboardManager::handle_squirrel_text_input_callback(const SDL_TextInputEvent& event)
{
  ssq::VM& ssq_vm = SquirrelVirtualMachine::current()->get_vm();
  try
  {
    if (ssq_vm.hasEntry("sector"))
    { 
      const auto& sector_table = ssq_vm.findTable("sector");

      if (sector_table.hasEntry("on_text_input"))
      {
        ssq::Function function = sector_table.findFunc("on_text_input");
        int param_count = function.getNumOfParams().second;
        if (param_count == 1)
        {
          ssq_vm.callFunc(function, ssq_vm, std::string(event.text));
        }
      }
    }
  }
  catch(std::exception& ex)
  {
    log_warning << ex.what() << std::endl;
  }
}
