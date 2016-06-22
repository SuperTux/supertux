//  DartTrap - Shoots a Dart at regular intervals
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_BADGUY_DARTTRAP_HPP
#define HEADER_SUPERTUX_BADGUY_DARTTRAP_HPP

/**
 * Badguy "DartTrap" - Shoots a Dart at regular intervals
 */
class DartTrap : public BadGuy
{
public:
  DartTrap(const ReaderMapping& reader);

  void initialize();
  void activate();
  void active_update(float elapsed_time);
  HitResponse collision_player(Player& player, const CollisionHit& hit);
  std::string get_class() const {
    return "darttrap";
  }
  std::string get_display_name() const {
    return _("Dart trap");
  }

  ObjectSettings get_settings();
  void after_editor_set();

protected:
  enum State {
    IDLE, LOADING
  };

  void load(); /**< load a shot */
  void fire(); /**< fire a shot */

private:
  float initial_delay; /**< time to wait before firing first shot */
  float fire_delay; /**< reload time */
  int ammo; /**< ammo left (-1 means unlimited) */

  State state; /**< current state */
  Timer fire_timer; /**< time until new shot is fired */
};

#endif

/* EOF */
