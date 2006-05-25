//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <memory>
#include <string>
#include "object/moving_sprite.hpp"
#include "object/path.hpp"
#include "object/path_walker.hpp"
#include "script_interface.hpp"

/**
 * This class is the base class for platforms that tux can stand on
 */
class Platform : public MovingSprite, public ScriptInterface
{
public:
  Platform(const lisp::Lisp& reader);
  Platform(const Platform& platform);
  virtual Platform* clone() const { return new Platform(*this); }

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual void update(float elapsed_time);
  const Vector& get_speed() const
  {
    return speed;
  }

  /** Move platform until at given node, then stop */
  void goto_node(int node_no);

  /** Start moving platform */
  void start_moving();

  /** Stop platform at next node */
  void stop_moving();

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

private:
  std::string name; /**< user-defined name for use in scripts or empty string if not scriptable */
  std::auto_ptr<Path> path;
  std::auto_ptr<PathWalker> walker;
  Vector speed;

};

#endif

