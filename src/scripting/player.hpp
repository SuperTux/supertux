//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_PLAYER_HPP
#define HEADER_SUPERTUX_SCRIPTING_PLAYER_HPP

#ifndef SCRIPTING_API
#include <string>

#include "scripting/game_object.hpp"

class Player;
#endif

namespace scripting {

/**
 * @summary This module contains methods controlling the player. (No, SuperTux doesn't use mind control. ""Player"" refers to the type of the player object.)
 * @instances The first player can be accessed using ""Tux"", or ""sector.Tux"" from the console.
              All following players (2nd, 3rd, etc...) can be accessed by ""Tux{index}"".
              For example, to access the 2nd player, use ""Tux1"" (or ""sector.Tux1"" from the console).
 */
class Player final
#ifndef SCRIPTING_API
  : public GameObject<::Player>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  Player(const Player&) = delete;
  Player& operator=(const Player&) = delete;
#endif

public:
  /**
   * Gives Tux the specified bonus unless Tux’s current bonus is superior.
   * @param string $bonus Can be "grow", "fireflower", "iceflower", "airflower" or "earthflower" at the moment.
   */
  bool add_bonus(const std::string& bonus);
  /**
   * Gives Tux the specified bonus.
   * @param string $bonus Can be "grow", "fireflower", "iceflower", "airflower" or "earthflower" at the moment.
   */
  bool set_bonus(const std::string& bonus);
  /**
   * Gets Tux's current bonus.
   */
  std::string get_bonus() const;

  /**
   * Gives the player a number of coins.${SRG_TABLENEWPARAGRAPH}
   * If count is a negative amount of coins, that number of coins will be taken
   * from the player (until the number of coins the player has is 0, when it
   * will stop changing).
   * @param int $count
   */
  void add_coins(int count);
  /**
   * Returns the number of coins the player currently has.
   */
  int get_coins() const;
  /**
   * Make Tux invincible for a short amount of time.
   */
  void make_invincible();
  /**
   * Deactivate user/scripting input for Tux.
     Carried items like trampolines won't be dropped.
   */
  void deactivate();
  /**
   * Give control back to user/scripting.
   */
  void activate();
  /**
   * Makes Tux walk.
   * @param float $speed
   */
  void walk(float speed);
  /**
   * Face Tux in the proper direction.
   * @param bool $right Set to ""true"" to make Tux face right, ""false"" to face left.
   */
  void set_dir(bool right);
  /**
   * Set Tux visible or invisible.
   * @param bool $visible
   */
  void set_visible(bool visible);
  /**
   * Returns ""true"" if Tux is currently visible (has not been set invisible by the ""set_visible()"" method).
   */
  bool get_visible() const;

  /**
   * Hurts Tux.
   * @param bool $completely If true, he will be killed even if he had "grow" or a superior bonus.
   */
  void kill(bool completely);

  /**
   * Switches ghost mode on/off. Lets Tux float around and through solid objects.
   * @param bool $enable
   */
  void set_ghost_mode(bool enable);

  /**
   * Returns whether ghost mode is currently enabled.
   */
  bool get_ghost_mode() const;

  /**
   * Start kick animation.
   */
  void kick();

  /**
   * Play cheer animation.${SRG_TABLENEWPARAGRAPH}
   * This might need some space and behave in an unpredictable way. It's best to use this at level end.
   */
  void do_cheer();

  /**
   * Makes Tux duck down, if possible. Won't last long, as long as input is enabled.
   */
  void do_duck();

  /**
   * Makes Tux stand back up, if possible.
   */
  void do_standup();

  /**
   * Makes Tux do a backflip, if possible.
   */
  void do_backflip();

  /**
   * Makes Tux jump in the air, if possible.
   * @param float $yspeed
   */
  void do_jump(float yspeed);

  /**
   * Orders the current ""GameSession"" to start a sequence.
   * @param string $sequence_name One of “stoptux”, “endsequence” or “fireworks”.
   */
  void trigger_sequence(const std::string& sequence_name);

  /**
   * Uses a scriptable controller for all user input (or restores controls).
   * @param bool $enable
   */
  void use_scripting_controller(bool enable);

  /**
   * Instructs the scriptable controller to press or release a button.
   * @param string $control Can be “left”, “right”, “up”, “down”, “jump”, “action”, “start”, “escape”,
      “menu-select”, “menu-select-space”, “menu-back”, “remove”, “cheat-menu”, “debug-menu”, “console”,
      “peek-left”, “peek-right”, “peek-up” or “peek-down”.
   * @param bool $pressed
   */
  void do_scripting_controller(const std::string& control, bool pressed);

  /**
   * Returns whether the player is carrying a certain object.
   * @param string $name Name of the portable object to check for.
   */
   bool has_grabbed(const std::string& name) const;

  /**
   * Returns Tux’s velocity in X direction.
   */
  float get_velocity_x() const;
  /**
   * Returns Tux’s velocity in Y direction.
   */
  float get_velocity_y() const;

  /**
   * Gets the X coordinate of the player.
   */
  float get_x() const;
  /**
   * Gets the Y coordinate of the player.
   */
  float get_y() const;

  /**
   * Sets the position of the player to a programmable/variable position.
   */
  void set_pos(float x, float y);

  /**
   * Gets the player's current action/animation.
   */
  std::string get_action() const;

  /**
   * Gets whether the current input on the keyboard/controller/touchpad has been pressed.
   * @param string $input Can be “left”, “right”, “up”, “down”, “jump”, “action”, “start”, “escape”,
      “menu-select”, “menu-select-space”, “menu-back”, “remove”, “cheat-menu”, “debug-menu”, “console”,
      “peek-left”, “peek-right”, “peek-up” or “peek-down”.
   */
  bool get_input_pressed(const std::string& input);

  /**
   * Gets whether the current input on the keyboard/controller/touchpad is being held.
   * @param string $input Valid values are listed above.
   */
  bool get_input_held(const std::string& input);
  /**
   * Gets whether the current input on the keyboard/controller/touchpad has been released.
   * @param string $input Valid values are listed above.
   */
  bool get_input_released(const std::string& input);
};

} // namespace scripting

#endif

/* EOF */
