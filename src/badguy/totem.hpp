//  $Id$
// 
//  SuperTux - "Totem" Badguy
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#ifndef __TOTEM_H__
#define __TOTEM_H__

#include "badguy.hpp"

/**
 * "Totem" Badguy - A variable-height stack of wooden blocks
 */
class Totem : public BadGuy
{
public:
  Totem(const lisp::Lisp& reader);
  ~Totem();

  void activate();
  void active_update(float elapsed_time);
  void write(lisp::Writer& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);

protected:
  bool stay_on_platform; /**< change direction before falling off a ledge */

  Totem* carrying; /**< Totem we are currently carrying (or 0) */
  Totem* carried_by; /**< Totem by which we are currently carried (or 0) */

  bool collision_squished(Player& player);
  void kill_fall();

  void jump_on(Totem* target); /**< jump on target */
  void jump_off(); /**< jump off current base */

  void synchronize_with(Totem* baseTotem); /**< synchronize position and movement with baseTotem */
};

#endif

