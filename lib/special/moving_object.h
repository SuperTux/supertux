//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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

#ifndef SUPERTUX_MOVING_OBJECT_H
#define SUPERTUX_MOVING_OBJECT_H

#include "special/base.h"
#include "special/game_object.h"
#include "math/vector.h"
//#include "rectangle.h"

namespace SuperTux
  {

  /**
   * Base class for all dynamic/moving game objects. This class contains things
   * for handling the bounding boxes and collision feedback.
   */
  class MovingObject : public GameObject
    {
    public:
      MovingObject();
      virtual ~MovingObject();

      /** this function is called when the object collided with any other object
       */
      virtual void collision(const MovingObject& other_object,
                             int collision_type) = 0;

      Vector get_pos() const
        {
          return Vector(base.x, base.y);
        }

      base_type base;
      base_type old_base;

    protected:
#if 0 // this will be used in my collision detection rewrite later
      /// the current position of the object
      Vector pos;
      /// the position we want to move until next frame
      Vector new_pos;
      /// the bounding box relative to the current position
      Rectangle bounding_box;
#endif
    };

} //namespace SuperTux

#endif /*SUPERTUX_MOVING_OBJECT_H*/

