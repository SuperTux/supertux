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
#ifndef SUPERTUX_TRIGGER_BASE_H
#define SUPERTUX_TRIGGER_BASE_H

#include "special/moving_object.h"
#include "math/rectangle.h"
#include "special/sprite.h"

class Player;
using namespace SuperTux;

/** This class is the base class for all objects you can interact with in some
 * way. There are several interaction types defined like touch and activate
 */
class TriggerBase : public MovingObject
{
public:
  enum EventType {
    EVENT_TOUCH, EVENT_ACTIVATE
  };
  
  TriggerBase();
  ~TriggerBase();

  void action(float elapsed_time);
  void draw(DrawingContext& context);
  HitResponse collision(GameObject& other, const CollisionHit& hit);
  
  /**
   * Receive trigger events
   */
  virtual void event(Player& player, EventType type) = 0;

private:
  Sprite* sprite;
  bool lasthit;
  bool hit;
};

#endif /*SUPERTUX_INTERACTIVE_OBJECT_H*/

