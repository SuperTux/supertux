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
#include "physic.h"
#include "collision.h"

enum ObjectType { OBJ_NONE, OBJ_BADGUY, OBJ_TRAMPOLINE };

template <class T>
struct ObjectData
{
  int x;
  int y;
  ObjectType type;
  T type_specific;

  ObjectData(ObjectData* pobject)
    : x((int)pobject->x), y((int)pobject->y), type(pobject->type), type_specific(pobject->type_specific) {};
  ObjectData(int x_, int y_, ObjectType type_, T type_specific_) 
    : x(x_), y(y_), type(type_), type_specific(type_specific_) {};

  ObjectData()
    : x(0), y(0), type(OBJ_NONE), type_specific() {};
};

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


/* Trampoline */
struct TrampolineData
{
  int power;
};

class Trampoline : public GameObject
{
 public:
  void init(float x, float y);
  void action(double frame_ratio);
  void draw();
  std::string type() { return "Trampoline"; };

  Trampoline(ObjectData<TrampolineData> data)
  {
    power = data.type_specific.power;

    init(data.x, data.y);
  };

  void collision(void *p_c_object, int c_object, CollisionType type);

  Physic physic;

 private:
  int power;
  unsigned int frame;
};


void load_object_gfx();

#endif 

/* Local Variables: */
/* mode:c++ */
/* End: */
