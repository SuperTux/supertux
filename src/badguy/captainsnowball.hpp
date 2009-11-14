//  SuperTux
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __CAPTAINSNOWBALL_H__
#define __CAPTAINSNOWBALL_H__

#include "walking_badguy.hpp"

class CaptainSnowball : public WalkingBadguy
{
public:
  CaptainSnowball(const lisp::Lisp& reader);
  CaptainSnowball(const Vector& pos, Direction d);

  virtual CaptainSnowball* clone() const { return new CaptainSnowball(*this); }
  virtual void active_update(float elapsed_time);
  void collision_solid(const CollisionHit& hit);

  bool might_climb(int width, int height);

protected:
  bool collision_squished(GameObject& object);

};

#endif
