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

#ifndef SUPERTUX_INTERACTIVE_OBJECT_H
#define SUPERTUX_INTERACTIVE_OBJECT_H

#include "game_object.h"
#include "type.h"

enum InteractionType
{
  INTERACTION_TOUCH, INTERACTION_ACTIVATE // more to come
};

/** This class is the base class for all objects you can interact with in some
 * way. There are several interaction types defined like touch and activate
 */
class InteractiveObject : public GameObject
{
public:
  InteractiveObject();
  virtual ~InteractiveObject();

  /** this function is called when an interaction has taken place */
  virtual void interaction(InteractionType type) = 0;

  const base_type& get_area() const
  { return area; }

  void set_area(float x, float y)
  { area.x = x; area.y = y; }

protected:
  base_type area;
};

#endif /*SUPERTUX_INTERACTIVE_OBJECT_H*/

