//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>,
//           2007,2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "control/input_manager.hpp"

#include "control/game_controller_manager.hpp"
#include "control/joystick_config.hpp"
#include "control/joystick_manager.hpp"
#include "control/keyboard_manager.hpp"
#include "control/remote_controller.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/savegame.hpp"
#include "supertux/sector.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"

static constexpr int MAX_PLAYERS = 4;

InputManager::InputManager(KeyboardConfig& keyboard_config,
                           JoystickConfig& joystick_config) :
  m_controllers(),
  m_use_game_controller(joystick_config.m_use_game_controller),
  keyboard_manager(new KeyboardManager(this, keyboard_config)),
  joystick_manager(new JoystickManager(this, joystick_config)),
  game_controller_manager(new GameControllerManager(this)),
  m_uses_keyboard()
{
  m_controllers.push_back(std::make_unique<Controller>());
}

InputManager::~InputManager()
{
}

const Controller&
InputManager::get_controller(int player_id) const
{
  return *m_controllers[player_id];
}

Controller&
InputManager::get_controller(int player_id)
{
  return *m_controllers[player_id];
}

bool
InputManager::can_add_user() const
{
  return !m_remote && (get_num_users() < MAX_PLAYERS || g_config->multiplayer_no_limit);
}

void
InputManager::use_game_controller(bool v)
{
  m_use_game_controller = v;

  // The 'rebinding' here is kind of a hack imo, but this is better than this
  // option just not working at all...
  if (m_use_game_controller)
    game_controller_manager->rebind_controllers();
  else
    joystick_manager->rebind_joysticks();

}

void
InputManager::update()
{
  for (auto& controller : m_controllers)
    if (controller.get() != m_remote) controller->update();
}

void
InputManager::reset()
{
  for (auto& controller : m_controllers)
    controller->reset();
}

void
InputManager::process_event(const SDL_Event& event)
{
  switch (event.type) {
    case SDL_EVENT_TEXT_INPUT:
      keyboard_manager->process_text_input_event(event.text);
      break;

    case SDL_EVENT_KEY_UP:
    case SDL_EVENT_KEY_DOWN:
      keyboard_manager->process_key_event(event.key);
      break;

    case SDL_EVENT_JOYSTICK_AXIS_MOTION:
      if (!m_use_game_controller) joystick_manager->process_axis_event(event.jaxis);
      break;

    case SDL_EVENT_JOYSTICK_HAT_MOTION:
      if (!m_use_game_controller) joystick_manager->process_hat_event(event.jhat);
      break;

    case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
    case SDL_EVENT_JOYSTICK_BUTTON_UP:
      if (!m_use_game_controller) joystick_manager->process_button_event(event.jbutton);
      break;

    case SDL_EVENT_JOYSTICK_ADDED:
      joystick_manager->on_joystick_added(event.jdevice.which);
      break;

    case SDL_EVENT_JOYSTICK_REMOVED:
      joystick_manager->on_joystick_removed(event.jdevice.which);
      break;

    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
      if (m_use_game_controller) game_controller_manager->process_axis_event(event.gaxis);
      break;

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
      if (m_use_game_controller) game_controller_manager->process_button_event(event.gbutton);
      break;

    case SDL_EVENT_GAMEPAD_BUTTON_UP:
      if (m_use_game_controller) game_controller_manager->process_button_event(event.gbutton);
      break;

    case SDL_EVENT_GAMEPAD_ADDED:
      log_debug << "SDL_EVENT_GAMEPAD_ADDED" << std::endl;
      game_controller_manager->on_controller_added(event.gdevice.which);
      break;

    case SDL_EVENT_GAMEPAD_REMOVED:
      log_debug << "SDL_EVENT_GAMEPAD_REMOVED" << std::endl;
      game_controller_manager->on_controller_removed(event.gdevice.which);
      break;

    case SDL_EVENT_GAMEPAD_REMAPPED:
      log_debug << "SDL_EVENT_GAMEPAD_REMAPPED" << std::endl;
      break;

    default:
      break;
  }
}

void
InputManager::push_user()
{
  if (!can_add_user())
    return;

  m_controllers.push_back(std::make_unique<Controller>());
}

void
InputManager::pop_user()
{
  if (m_controllers.size() <= 1)
    throw std::runtime_error("Attempt to pop the first player's controller");

  const int id = get_num_users() - 1;
  if (is_remote(id)) return;
  // Destroy Players (including their temporary script-controller pointers)
  // before destroying the controller they borrow. Menu removal is deferred.
  if (GameSession::current())
    GameSession::current()->on_player_removed(id);
  on_player_removed(id);

  m_controllers.pop_back();
}

void
InputManager::on_player_removed(int player_id)
{
  joystick_manager->on_player_removed(player_id);
  game_controller_manager->on_player_removed(player_id);
}

bool
InputManager::has_corresponsing_controller(int player_id) const
{
  if (is_remote(player_id)) return true;
  if (m_use_game_controller)
  {
    return game_controller_manager->has_corresponding_game_controller(player_id);
  }
  else
  {
    return joystick_manager->has_corresponding_joystick(player_id);
  }
}

bool
InputManager::is_local(int player_id) const
{
  return player_id >= 0 && player_id < get_num_users() && !is_remote(player_id);
}

bool
InputManager::is_remote(int player_id) const
{
  return m_remote && player_id == 1;
}

int
InputManager::persistent_users() const
{
  return get_num_users() - (m_remote ? 1 : 0);
}

bool
InputManager::reserve_remote()
{
  if (m_remote) return true;
  if (get_num_users() != 1) return false; // proof is exactly one local + one remote
  auto remote = std::make_unique<RemoteController>();
  m_remote = remote.get();
  m_controllers.push_back(std::move(remote));
  return true;
}

void
InputManager::reset_remote()
{
  if (m_remote) m_remote->reset();
}

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
// Callbacks only write bounded JS queues. All ownership/state changes happen
// here, on the game thread; JavaScript never retains C++ object pointers.
EM_JS(int, browser_coop_poll, (uint32_t* fields), {
  var item = Module.supertuxCoop && Module.supertuxCoop.poll();
  if (!item) return 0;
  HEAPU32.set(item, fields >>> 2);
  return 1;
});
EM_JS(void, browser_coop_status, (int reserved, int enabled, uint32_t generation, uint32_t sequence), {
  if (Module.supertuxCoop) Module.supertuxCoop.engineStatus(reserved, enabled, generation, sequence);
});
#endif

void
InputManager::update_remote(bool gameplay)
{
  if (m_remote) m_remote->set_enabled(gameplay && m_remote_connected);
#ifdef __EMSCRIPTEN__
  uint32_t fields[4];
  bool rejected = false;
  for (size_t i = 0; i < RemoteController::QUEUE_LIMIT + 1 && browser_coop_poll(fields); ++i)
  {
    if (fields[0] == 1) // attach only before a playable level
    {
      const auto screen = ScreenManager::current();
      if (screen && !screen->get_screen_stack().empty() &&
          dynamic_cast<TitleScreen*>(screen->get_screen_stack().back().get()))
        m_remote_connected = reserve_remote();
      else
        m_remote_connected = false;
      rejected = !m_remote_connected;
      if (m_remote) m_remote->set_enabled(gameplay && m_remote_connected);
    }
    else if (fields[0] == 2 && m_remote)
      m_remote->submit(fields[1], fields[2], fields[3]);
    else if (fields[0] == 3)
    {
      m_remote_connected = false;
      if (m_remote) m_remote->set_enabled(false);
    }
    else if (fields[0] == 4) reset_remote();
    else if (fields[0] == 5 && fields[1] < 2) // host-only diagnostic level selection
    {
      const auto screen = ScreenManager::current();
      if (screen && !screen->get_screen_stack().empty() &&
          dynamic_cast<TitleScreen*>(screen->get_screen_stack().back().get()))
      {
        const bool forest = fields[1] == 1;
        const auto world = World::from_directory(forest ? "levels/world2" : "levels/world1");
        MenuManager::instance().clear_menu_stack();
        GameManager::current()->start_level(*world,
          forest ? "tux_builder.stl" : "welcome_antarctica.stl", std::nullopt, true);
      }
    }
  }
#endif
  // Apply after every Controller has captured its previous state, before the
  // screen/sector simulation. RemoteController alone advances its own state.
  if (m_remote) m_remote->update();
#ifdef __EMSCRIPTEN__
  browser_coop_status(rejected ? -2 : (m_remote_connected ? 1 : (m_remote ? -1 : 0)), m_remote && m_remote->enabled(),
                      m_remote ? m_remote->generation() : 0, m_remote ? m_remote->sequence() : 0);
#endif
}
