//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
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

#ifndef SUPERTUX_SPECIAL_H
#define SUPERTUX_SPECIAL_H

#include "SDL.h"

#include "bitmask.h"
#include "type.h"
#include "screen/surface.h"
#include "collision.h"
#include "player.h"
#include "physic.h"

/* Upgrade types: */

enum UpgradeKind {
  UPGRADE_GROWUP,
  UPGRADE_FIREFLOWER,
  UPGRADE_ICEFLOWER,
  UPGRADE_HERRING,
  UPGRADE_1UP
};

void load_special_gfx();
void free_special_gfx();

class Upgrade : public MovingObject
{
public:
  UpgradeKind kind;
  Direction  dir;
  Physic physic;

  Upgrade(const Vector& pos, Direction dir, UpgradeKind kind);
  virtual ~Upgrade();
  
  virtual void action(float frame_ratio);
  virtual void draw(DrawingContext& context);

  virtual void collision(const MovingObject& other, int);
  void collision(void* p_c_object, int c_object, CollisionType type);

private:
  void bump(Player* player);
};

enum BulletsKind {
  FIRE_BULLET,
  ICE_BULLET
};

class Bullet : public MovingObject
{
public:
  Bullet(const Vector& pos, float xm, int dir,
      int kind);
  
  virtual void action(float frame_ratio);
  virtual void draw(DrawingContext& context);
  void collision(int c_object);

  virtual void collision(const MovingObject& other_object, int type);

  int kind;        
  
private:
  int life_count;
  Physic physic;
};

#endif /*SUPERTUX_SPECIAL_H*/
