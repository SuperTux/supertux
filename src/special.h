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

#include <SDL.h>
#include "bitmask.h"
#include "type.h"
#include "texture.h"
#include "collision.h"
#include "player.h"
#include "physic.h"

/* Upgrade types: */

enum UpgradeKind {
  UPGRADE_GROWUP,
  UPGRADE_ICEFLOWER,
  UPGRADE_HERRING,
  UPGRADE_1UP
};

void load_special_gfx();
void free_special_gfx();

class Upgrade : public GameObject
{
public:
  UpgradeKind kind;
  Direction  dir;
  Physic physic;

  void init(float x, float y, Direction dir, UpgradeKind kind);
  void action(double frame_ratio);
  void draw();
  void collision(void* p_c_object, int c_object, CollisionType type);
  std::string type() { return "Upgrade"; };
  
  ~Upgrade() {};

private:
  /** removes the Upgrade from the global upgrade list. Note that after this
   * call the class doesn't exist anymore! So don't use any member variables
   * anymore then
   */
  void remove_me();

  void bump(Player* player);
};

class Bullet : public GameObject
{
 public:
  int life_count;
  base_type base;
  base_type old_base;
  
  void init(float x, float y, float xm, Direction dir);
  void action(double frame_ratio);
  void draw();
  void collision(int c_object);
  std::string type() { return "Bullet"; };

private:
  /** removes the Upgrade from the global upgrade list. Note that after this
   * call the class doesn't exist anymore! So don't use any member variables
   * anymore then
   */
  void remove_me();
};

#endif /*SUPERTUX_SPECIAL_H*/
