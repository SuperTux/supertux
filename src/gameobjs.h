//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 SuperTux Development Team, see AUTHORS for details
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

#ifndef SUPERTUX_GAMEOBJS_H
#define SUPERTUX_GAMEOBJS_H

#include "type.h"
#include "texture.h"
#include "timer.h"
#include "scene.h"

/* Bounciness of distros: */
#define NO_BOUNCE 0
#define BOUNCE 1

class BouncyDistro
{
 public:
  base_type base;
  
  void init(float x, float y);
  void action(double frame_ratio);
  void draw(); 
};

extern Surface* img_distro[4];

#define BOUNCY_BRICK_MAX_OFFSET 8
#define BOUNCY_BRICK_SPEED 0.9

class Tile;

class BrokenBrick
{
 public:
  base_type base;
  Timer timer;
  Tile* tile;

  void init(Tile* tile, float x, float y, float xm, float ym);
  void action(double frame_ratio);
  void draw();
};

class BouncyBrick
{
 public:
  float offset;
  float offset_m;
  int shape;
  base_type base;

  void init(float x, float y);
  void action(double frame_ratio);
  void draw();
};

class FloatingScore
{
 public:
  int value;
  Timer timer;
  base_type base;
  
  void init(float x, float y, int s);
  void action(double frame_ratio);
  void draw();
};

#endif 

/* Local Variables: */
/* mode:c++ */
/* End: */
