//  $Id$
//
//  SuperTux
//  Copyright (C) 2005 Marek Moeckel <wansti@gmx.de>
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
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "moving_object.hpp"
#include "sprite/sprite.hpp"
#include "object/path.hpp"

/**
 * This class is the base class for platforms that tux can stand on
 */
class Platform : public MovingObject
{
public:
  Platform(const lisp::Lisp& reader);
  ~Platform();

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Sprite* sprite;
  Path* path;
  Vector path_offset;
};

#endif

