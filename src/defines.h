//  $Id$
// 
//  SuperTux
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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
#ifndef SUPERTUX_DEFINES_H
#define SUPERTUX_DEFINES_H

#include <config.h>

enum Direction { LEFT = 0, RIGHT = 1 };

/* Direction (keyboard/joystick) states: */

#define UP 0
#define DOWN 1

/* Dying types: */

/* ---- NO 0 */
enum DyingType {
  DYING_NOT = 0,
  DYING_SQUISHED = 1,
  DYING_FALLING = 2
};

/* Speed constraints: */
#define MAX_WALK_XM 230
#define MAX_RUN_XM 320
#define MAX_LIVES 99

#define WALK_SPEED 100

/* gameplay related defines */

#define START_LIVES 4

#define MAX_FIRE_BULLETS 2
#define MAX_ICE_BULLETS  1
#define FROZEN_TIME 3.0

#define WALK_ACCELERATION_X 300
#define RUN_ACCELERATION_X 400

#define SKID_XM 200
#define SKID_TIME .3

#endif /*SUPERTUX_DEFINES_H*/

