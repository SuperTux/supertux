// C Interface: collision
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@web.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_COLLISION_H
#define SUPERTUX_COLLISION_H

#include "type.h"

/* Collision objects */
enum
{
  CO_BULLET,
  CO_BADGUY,
  CO_PLAYER
};

enum CollisionType {
    COLLISION_NORMAL,
    COLLISION_BUMP,
    COLLISION_SQUISH
};

bool rectcollision(base_type* one, base_type* two);
bool rectcollision_offset(base_type* one, base_type* two, float off_x, float off_y);
void collision_swept_object_map(base_type* old, base_type* current);
bool collision_object_map(base_type* object);

/* Checks for all possible collisions.
   And calls the collision_handlers, which the collision_objects provide for this case (or not). */
void collision_handler();

#endif /*SUPERTUX_COLLISION_H*/

