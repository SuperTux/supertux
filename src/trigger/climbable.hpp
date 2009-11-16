//  SuperTux - Climbable area
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_TRIGGER_CLIMBABLE_HPP
#define HEADER_SUPERTUX_TRIGGER_CLIMBABLE_HPP

#include "supertux/resources.hpp"
#include "supertux/timer.hpp"
#include "trigger/trigger_base.hpp"
#include "util/reader_fwd.hpp"
#include "video/drawing_context.hpp"

class Player;

class Climbable : public TriggerBase
{
  static Color text_color;
public:
  Climbable(const Reader& reader);
  Climbable(const Rect& area);
  ~Climbable();

  void event(Player& player, EventType type);
  void update(float elapsed_time);
  void draw(DrawingContext& context);

  /**
   * returns true if the player is within bounds of the Climbable
   */
  bool may_climb(Player& player);

protected:
  Player* climbed_by; /**< set to player who's currently climbing us, null if nobody is */
  Timer activate_try_timer; /**< try to correct mis-alignment while this timer runs */

private:
  Climbable(const Climbable&);
  Climbable& operator=(const Climbable&);
};

#endif

/* EOF */
