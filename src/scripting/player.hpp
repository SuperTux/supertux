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
   * Set tux bonus.
   * This can be "grow", "fireflower" or "iceflower" at the moment
   */
  bool add_bonus(const std::string& bonus);
  /**
   * Replaces the Tux's bonus with another bonus.
   * This can be "grow", "fireflower" or "iceflower" at the moment
   */
  bool set_bonus(const std::string& bonus);

  std::string get_bonus() const;

  int get_bonus_id() const;

  /**
   * Give tux more coins
   *
   * If count is a negative amount of coins, that number of coins will be taken
   * from the player (until the number of coins the player has is 0, when it
   * will stop changing).
   */
  void add_coins(int count);
  /**
   * Returns the number of coins the player currently has.
   */
  int get_coins() const;
  /**
   * Make tux invincible for a short amount of time
   */
  void make_invincible();
  /**
   * Deactivate user/scripting input for Tux
   */
  void deactivate();
  /**
   * Give control back to user/scripting
   */
  void activate();
  /**
   * Make Tux walk
   */
  void walk(float speed);
  /**
   * Face Tux in the proper direction
   */
  void set_dir(bool right);
  /**
   * Set player visible or invisible
   */
  void set_visible(bool visible);
  /**
   * returns true if Tux is currently visible (that is he was not set
   * invisible by the set_visible method)
   */
  bool get_visible() const;

  /**
   * Hurts Tux, if completely=true then he will be killed even
   * if he had grow or fireflower bonus
   */
  void kill(bool completely);

  /**
   * Switches ghost mode on/off.
   * Lets Tux float around and through solid objects.
   */
  void set_ghost_mode(bool enable);

  /**
   * Returns whether ghost mode is currently enabled
   */
  bool get_ghost_mode() const;

  /**
   * start kick animation
   */
  void kick();

  /**
   * play cheer animation.
   * This might need some space and behave in an unpredictable way. Best to use this at level end.
   */
  void do_cheer();

  /**
   * duck down if possible.
   * this won't last long as long as input is enabled.
   */
  void do_duck();

  /**
   * stand back up if possible.
   */
  void do_standup();

  /**
   * do a backflip if possible.
   */
  void do_backflip();

  /**
   * jump in the air if possible
   * sensible values for yspeed are negative - unless we want to jump into the ground of course
   */
  void do_jump(float yspeed);

  /**
   * Orders the current GameSession to start a sequence
   */
  void trigger_sequence(const std::string& sequence_name);

  /**
   * Uses a scriptable controller for all user input (or restores controls)
   */
  void use_scripting_controller(bool use_or_release);

  /**
   * Check whether player is carrying a certain object
   * @param name Name of the Portable object to check for
   */
   bool has_grabbed(const std::string& name) const;

  /**
   * Instructs the scriptable controller to press or release a button
   */
  void do_scripting_controller(const std::string& control, bool pressed);

  float get_velocity_x() const;
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
};

} // namespace scripting

#endif

/* EOF */
