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

#include <string>

namespace Scripting
{

class Player
{
public:
#ifndef SCRIPTING_API
  virtual ~Player()
  {}
#endif

  /**
   * Set tux bonus.
   * This can be "grow", "fireflower" or "iceflower" at the moment
   */
  virtual bool add_bonus(const std::string& bonus) = 0;
  /**
   * Give tux more coins
   */
  virtual void add_coins(int count) = 0;
  /**
   * Make tux invincible for a short amount of time
   */
  virtual void make_invincible() = 0;
  /**
   * Deactivate user/scripting input for Tux
   */
  virtual void deactivate() = 0;
  /**
   * Give control back to user/scripting
   */
  virtual void activate() = 0;
  /**
   * Make Tux walk
   */
  virtual void walk(float speed) = 0;
  /**
   * Set player visible or invisible
   */
  virtual void set_visible(bool visible) = 0;
  /**
   * returns true if the player is currently visible (that is he was not set
   * invisible by the set_visible method)
   */
  virtual bool get_visible() = 0;

  /**
   * Hurts a player, if completely=true then the player will be killed even
   * if he had grow or fireflower bonus
   */
  virtual void kill(bool completely) = 0;

  /**
   * Switches ghost mode on/off.
   * Lets Tux float around and through solid objects.
   */
  virtual void set_ghost_mode(bool enable) = 0;

  /**
   * Returns whether ghost mode is currently enabled
   */
  virtual bool get_ghost_mode() = 0;

  /**
   * play cheer animation.
   * This might need some space and behave in an unpredictable way. Best to use this at level end.
   */
  virtual void do_cheer() = 0;

  /**
   * duck down if possible.
   * this won't last long as long as input is enabled.
   */
  virtual void do_duck() = 0;

  /**
   * stand back up if possible.
   */
  virtual void do_standup() = 0;

  /**
   * do a backflip if possible.
   */
  virtual void do_backflip() = 0;

  /**
   * jump in the air if possible
   * sensible values for yspeed are negative - unless we want to jump into the ground of course
   */
  virtual void do_jump(float yspeed) = 0;

  /**
   * Orders the current GameSession to start a sequence
   */
  virtual void trigger_sequence(std::string sequence_name) = 0;

  /**
   * Uses a scriptable controller for all user input (or restores controls)
   */
  virtual void use_scripting_controller(bool use_or_release) = 0;

  /**
   * Instructs the scriptable controller to press or release a button 
   */
  virtual void do_scripting_controller(std::string control, bool pressed) = 0;

};

}

#endif
