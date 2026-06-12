//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include <algorithm>

#include "control/input_manager.hpp"
#include "control/joystick_config.hpp"
#include "gui/menu_manager.hpp"
#include "object/player.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/game_session.hpp"
#include "supertux/savegame.hpp"
#include "util/log.hpp"

JoystickManager::JoystickManager(InputManager* parent_,
                                 std::map<int, JoystickConfig>& joystick_configs) :
  parent(parent_),
  m_joystick_configs(joystick_configs),
  min_joybuttons(),
  max_joybuttons(),
  max_joyaxis(),
  max_joyhats(),
  hat_state(0),
  wait_for_joystick(std::nullopt),
  joysticks()
{
}

JoystickManager::~JoystickManager()
{
  for (auto& joy : joysticks)
  {
    SDL_JoystickClose(joy.first);
  }
}

void
JoystickManager::on_joystick_added(int joystick_index)
{
  log_debug << "on_joystick_added(): " << joystick_index << std::endl;

  if (!parent->can_add_user())
    return;

  SDL_Joystick* joystick = SDL_JoystickOpen(joystick_index);
  if (!joystick)
  {
    log_warning << "failed to open joystick: " << joystick_index
                << ": " << SDL_GetError() << std::endl;
  }
  else
  {
    joysticks[joystick] = -1;

    if (min_joybuttons < 0 || SDL_JoystickNumButtons(joystick) < min_joybuttons)
      min_joybuttons = SDL_JoystickNumButtons(joystick);

    if (SDL_JoystickNumButtons(joystick) > max_joybuttons)
      max_joybuttons = SDL_JoystickNumButtons(joystick);

    if (SDL_JoystickNumAxes(joystick) > max_joyaxis)
      max_joyaxis = SDL_JoystickNumAxes(joystick);

    if (SDL_JoystickNumHats(joystick) > max_joyhats)
      max_joyhats = SDL_JoystickNumHats(joystick);

    if (!parent->m_use_game_controller && g_config->multiplayer_auto_manage_players)
    {
      int id = parent->get_num_users();
      for (int i = 0; i < parent->get_num_users(); i++)
      {
        if (!parent->has_corresponsing_controller(i) && !parent->m_uses_keyboard[i])
        {
          id = i;
          break;
        }
      }

      if (id == parent->get_num_users())
        parent->push_user();

      joysticks[joystick] = id;

      if (GameSession::current() && !GameSession::current()->get_savegame().is_title_screen() && id != 0)
      {
        GameSession::current()->on_player_added(id);
      }
    }
  }
}

void
JoystickManager::on_joystick_removed(int instance_id)
{
  log_debug << "on_joystick_removed: " << static_cast<int>(instance_id) << std::endl;

  auto it = std::find_if(joysticks.begin(), joysticks.end(), [instance_id] (decltype(joysticks)::const_reference pair) {
    return SDL_JoystickInstanceID(pair.first) == instance_id;
  });

  if (it != joysticks.end())
  {
    SDL_JoystickClose(it->first);

    auto deleted_player_id = it->second;

    joysticks.erase(it);

    if (!parent->m_use_game_controller && g_config->multiplayer_auto_manage_players
        && deleted_player_id != 0 && !parent->m_uses_keyboard[deleted_player_id] &&
        GameSession::current())
    {
      GameSession::current()->on_player_removed(deleted_player_id);
    }
  }
  else
  {
    log_debug << "Joystick was unplugged but was not initially detected: "
              << SDL_JoystickName(SDL_JoystickFromInstanceID(instance_id))
              << std::endl;
  }
}

void
JoystickManager::process_hat_event(const SDL_JoyHatEvent& jhat)
{
  Uint8 changed = hat_state ^ jhat.value;

  if (wait_for_joystick.has_value())
  {
    auto& cfg = m_joystick_configs[wait_for_joystick->player_id];

    if (changed & SDL_HAT_UP && jhat.value & SDL_HAT_UP)
      cfg.bind_joyhat(SDL_HAT_UP, wait_for_joystick->control);

    if (changed & SDL_HAT_DOWN && jhat.value & SDL_HAT_DOWN)
      cfg.bind_joyhat(SDL_HAT_DOWN, wait_for_joystick->control);

    if (changed & SDL_HAT_LEFT && jhat.value & SDL_HAT_LEFT)
      cfg.bind_joyhat(SDL_HAT_LEFT, wait_for_joystick->control);

    if (changed & SDL_HAT_RIGHT && jhat.value & SDL_HAT_RIGHT)
      cfg.bind_joyhat(SDL_HAT_RIGHT, wait_for_joystick->control);

    MenuManager::instance().refresh();
    wait_for_joystick = std::nullopt;
  }
  else
  {
    auto& cfg = get_config_for_joystick(jhat.which);

    if (changed & SDL_HAT_UP)
    {
      auto it = cfg.m_joy_hat_map.find(SDL_HAT_UP);
      if (it != cfg.m_joy_hat_map.end())
        set_joy_controls(jhat.which, it->second, (jhat.value & SDL_HAT_UP) != 0);
    }

    if (changed & SDL_HAT_DOWN)
    {
      auto it = cfg.m_joy_hat_map.find(SDL_HAT_DOWN);
      if (it != cfg.m_joy_hat_map.end())
        set_joy_controls(jhat.which, it->second, (jhat.value & SDL_HAT_DOWN) != 0);
    }

    if (changed & SDL_HAT_LEFT)
    {
      auto it = cfg.m_joy_hat_map.find(SDL_HAT_LEFT);
      if (it != cfg.m_joy_hat_map.end())
        set_joy_controls(jhat.which, it->second, (jhat.value & SDL_HAT_LEFT) != 0);
    }

    if (changed & SDL_HAT_RIGHT)
    {
      auto it = cfg.m_joy_hat_map.find(SDL_HAT_RIGHT);
      if (it != cfg.m_joy_hat_map.end())
        set_joy_controls(jhat.which, it->second, (jhat.value & SDL_HAT_RIGHT) != 0);
    }
  }

  hat_state = jhat.value;
}

void
JoystickManager::process_axis_event(const SDL_JoyAxisEvent& jaxis)
{
  if (g_config->ignore_joystick_axis)
    return;

  if (wait_for_joystick.has_value())
  {
    auto& cfg = m_joystick_configs[wait_for_joystick->player_id];
    if (abs(jaxis.value) > cfg.m_dead_zone) {
      if (jaxis.value < 0)
        cfg.bind_joyaxis(-(jaxis.axis + 1), wait_for_joystick->control);
      else
        cfg.bind_joyaxis(jaxis.axis + 1, wait_for_joystick->control);

      MenuManager::instance().refresh();
      wait_for_joystick = std::nullopt;
    }
  }
  else
  {
    auto& cfg = get_config_for_joystick(jaxis.which);

    // Split the axis into left and right, so that both can be
    // mapped separately (needed for jump/down vs up/down)
    int axis = jaxis.axis + 1;

    auto left = cfg.m_joy_axis_map.find(-axis);
    auto right = cfg.m_joy_axis_map.find(axis);

    if (left == cfg.m_joy_axis_map.end()) {
      // std::cout << "Unmapped joyaxis " << (int)jaxis.axis << " moved" << std::endl;
    } else {
      if (jaxis.value < -cfg.m_dead_zone)
        set_joy_controls(jaxis.which, left->second,  true);
      else
        set_joy_controls(jaxis.which, left->second, false);
    }

    if (right == cfg.m_joy_axis_map.end()) {
      // std::cout << "Unmapped joyaxis " << (int)jaxis.axis << " moved" << std::endl;
    } else {
      if (jaxis.value > cfg.m_dead_zone)
        set_joy_controls(jaxis.which, right->second, true);
      else
        set_joy_controls(jaxis.which, right->second, false);
    }
  }
}

void
JoystickManager::process_button_event(const SDL_JoyButtonEvent& jbutton)
{
  if (wait_for_joystick.has_value())
  {
    if (jbutton.state == SDL_PRESSED)
    {
      m_joystick_configs[wait_for_joystick->player_id].bind_joybutton(jbutton.button, wait_for_joystick->control);
      MenuManager::instance().refresh();
      parent->reset();
      wait_for_joystick = std::nullopt;
    }
  }
  else
  {
    auto& cfg = get_config_for_joystick(jbutton.which);
    auto i = cfg.m_joy_button_map.find(jbutton.button);
    if (i == cfg.m_joy_button_map.end()) {
      log_debug << "Unmapped joybutton " << static_cast<int>(jbutton.button) << " pressed" << std::endl;
    } else {
      set_joy_controls(jbutton.which, i->second, (jbutton.state == SDL_PRESSED));
    }
  }
}

void
JoystickManager::bind_next_event_to(int player_id, Control id)
{
  wait_for_joystick = PlayerControl{player_id, id};
}

JoystickConfig&
JoystickManager::get_config_for_joystick(SDL_JoystickID instance_id)
{
  SDL_Joystick* joystick = SDL_JoystickFromInstanceID(instance_id);
  auto it = joysticks.find(joystick);
  int player_id = (it != joysticks.end() && it->second >= 0) ? it->second : 0;

  if (m_joystick_configs.find(player_id) == m_joystick_configs.end())
    m_joystick_configs[player_id] = JoystickConfig();

  return m_joystick_configs[player_id];
}

void
JoystickManager::set_joy_controls(SDL_JoystickID joystick, Control id, bool value)
{
  auto it = joysticks.find(SDL_JoystickFromInstanceID(joystick));
  if (it == joysticks.end() || it->second < 0)
    return;

  if (get_config_for_joystick(joystick).m_jump_with_up_joy &&
      id == Control::UP)
  {
    parent->get_controller(it->second).set_control(Control::JUMP, value);
  }

  parent->get_controller(it->second).set_control(id, value);
}

void
JoystickManager::on_player_removed(int player_id)
{
  auto it2 = std::find_if(joysticks.begin(), joysticks.end(), [player_id](decltype(joysticks)::const_reference pair) {
    return pair.second == player_id;
  });
  if (it2 != joysticks.end())
  {
    it2->second = -1;
    // Try again, in case multiple controllers were bount to a player
    // Recursive call shouldn't go too deep except in hardcore scenarios
    on_player_removed(player_id);
  }
}

bool
JoystickManager::has_corresponding_joystick(int player_id) const
{
  return std::find_if(joysticks.begin(), joysticks.end(), [player_id](decltype(joysticks)::const_reference pair) {
    return pair.second == player_id;
  }) != joysticks.end();
}

int
JoystickManager::rumble(SDL_Joystick* controller) const
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
  if (g_config->multiplayer_buzz_controllers)
  {
#if SDL_VERSION_ATLEAST(2, 0, 18)
    if (SDL_JoystickHasRumble(controller))
    {
#endif
      // TODO: Rumble intensity setting (like volume)
      SDL_JoystickRumble(controller, 0xFFFF, 0xFFFF, 300);
#if SDL_VERSION_ATLEAST(2, 0, 18)
    }
    else
    {
      return 1;
    }
#endif
  }

  return 0;
#else
  return 2;
#endif
}

void
JoystickManager::bind_joystick(SDL_Joystick* controller, int player_id)
{
  joysticks[controller] = player_id;

  if (!g_config->multiplayer_multibind)
    for (auto& pair2 : joysticks)
      if (pair2.second == player_id && pair2.first != controller)
        pair2.second = -1;
}

void
JoystickManager::rebind_joysticks()
{
  int i = 0;
  for (const auto& joystick : joysticks)
  {
    assert(joystick.first != nullptr);
    if (joystick.second == -1)
      bind_joystick(joystick.first, i++);
  }
}
