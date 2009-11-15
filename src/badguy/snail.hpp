//  SuperTux - Badguy "Snail"
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

#ifndef HEADER_SUPERTUX_BADGUY_SNAIL_HPP
#define HEADER_SUPERTUX_BADGUY_SNAIL_HPP

#include "walking_badguy.hpp"

/**
 * Badguy "Snail" - a snail-like creature that can be flipped and tossed around at an angle
 */
class Snail : public WalkingBadguy
{
public:
  Snail(const lisp::Lisp& reader);
  Snail(const Vector& pos, Direction d);

  void initialize();
  void write(lisp::Writer& writer);
  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
  bool can_break();

  void active_update(float elapsed_time);

  virtual Snail* clone() const { return new Snail(*this); }

protected:
  bool collision_squished(GameObject& object);
  void be_normal(); /**< switch to state STATE_NORMAL */
  void be_flat(); /**< switch to state STATE_FLAT */
  void be_kicked(); /**< switch to state STATE_KICKED_DELAY */

private:
  enum State {
    STATE_NORMAL, /**< walking around */
    STATE_FLAT, /**< flipped upside-down */
    STATE_KICKED_DELAY, /**< short delay before being launched */
    STATE_KICKED /**< launched */
  };
  State state;
  Timer flat_timer; /**< wait time until flipping right-side-up again */
  Timer kicked_delay_timer; /**< wait time until switching from STATE_KICKED_DELAY to STATE_KICKED */
  int squishcount;
};

#endif
