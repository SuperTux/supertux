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

#ifndef SUPERTUX_GAMEOBJS_H
#define SUPERTUX_GAMEOBJS_H

#include "type.h"
#include "texture.h"
#include "timer.h"
#include "scene.h"

/* Bounciness of distros: */
#define NO_BOUNCE 0
#define BOUNCE 1

class BouncyDistro : public GameObject
{
 public:
  
  void init(float x, float y);
  void action(double frame_ratio);
  void draw(); 
  std::string type() { return "BouncyDistro"; };
};

extern Surface* img_distro[4];

#define BOUNCY_BRICK_MAX_OFFSET 8
#define BOUNCY_BRICK_SPEED 0.9

class Tile;

class BrokenBrick : public GameObject
{
 public:
  Timer timer;
  Tile* tile;

  void init(Tile* tile, float x, float y, float xm, float ym);
  void action(double frame_ratio);
  void draw();
  std::string type() { return "BrokenBrick"; };
};

class BouncyBrick : public GameObject
{
 public:
  float offset;
  float offset_m;
  int shape;

  void init(float x, float y);
  void action(double frame_ratio);
  void draw();
  std::string type() { return "BouncyBrick"; };
};

class FloatingScore : public GameObject
{
 public:
  int value;
  Timer timer;
  
  void init(float x, float y, int s);
  void action(double frame_ratio);
  void draw();
  std::string type() { return "FloatingScore"; };
};

#endif 

/* Local Variables: */
/* mode:c++ */
/* End: */
