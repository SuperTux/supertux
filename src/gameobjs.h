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

#include "special/base.h"
#include "video/surface.h"
#include "special/timer.h"
#include "scene.h"
#include "math/physic.h"
#include "collision.h"
#include "special/game_object.h"
#include "special/moving_object.h"
#include "serializable.h"
#include "utils/lispwriter.h"

/* Bounciness of distros: */
#define NO_BOUNCE 0
#define BOUNCE 1

namespace SuperTux {
class Sprite;
}

struct TileId;

class BouncyDistro : public GameObject
{
public:
  BouncyDistro(const Vector& pos);
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Vector position;
  float ym;
};

extern Surface* img_distro[4];

#define BOUNCY_BRICK_MAX_OFFSET 8
#define BOUNCY_BRICK_SPEED 0.9

class Tile;

class BrokenBrick : public GameObject
{
public:
  BrokenBrick(Tile* tile, const Vector& pos, const Vector& movement);

  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Timer timer;
  Tile* tile;
  Vector position;
  Vector movement;
};

class BouncyBrick : public GameObject
{
public:
  BouncyBrick(const Vector& pos);
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);
  
private:
  Vector position;
  float offset;   
  float offset_m;
  TileId& shape;      
};

class FloatingScore : public GameObject
{
public:
  FloatingScore(const Vector& pos, int s);
  
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Vector position;
  char str[10];
  Timer timer;  
};

extern Sprite *img_trampoline;

class Trampoline : public MovingObject, public Serializable
{
public:
  Trampoline(LispReader& reader);
  Trampoline(float x, float y);
 
  virtual void write(LispWriter& writer);
  virtual void action(float frame_ratio);
  virtual void draw(DrawingContext& context);

  virtual void collision(const MovingObject& other, int);
  void collision(void *p_c_object, int c_object, CollisionType type);

  Physic physic;
  enum { M_NORMAL, M_HELD } mode;

 private:
  float power;
  unsigned int frame;
};

extern Sprite *img_flying_platform;

class FlyingPlatform : public MovingObject, public Serializable
{
public:
  FlyingPlatform(LispReader& reader);
  FlyingPlatform(int x, int y);
 
  virtual void write(LispWriter& writer);
  virtual void action(float frame_ratio);
  virtual void draw(DrawingContext& context);

  virtual void collision(const MovingObject& other, int);
  void collision(void *p_c_object, int c_object, CollisionType type);

  float get_vel_x() { return vel_x; }
  float get_vel_y() { return vel_y; }

  Physic physic;
  enum { M_NORMAL, M_HELD } mode;

 private:
  std::vector<int> pos_x;
  std::vector<int> pos_y;
  float velocity;

  float vel_x, vel_y;  // calculated based in the velocity

  int point;
  bool move;
  unsigned int frame;
};

extern Sprite *img_smoke_cloud;

class SmokeCloud : public GameObject
{
public:
  SmokeCloud(const Vector& pos);
  
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Timer timer;
  Vector position;
};

class Particles : public GameObject
{
public:
  Particles(const Vector& epicenter, const Vector& velocity, const Vector& acceleration, int number, Color color, int size, int life_time);
  ~Particles();
  
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Color color;
  float size;
  Vector vel, accel;
  Timer timer;
  bool live_forever;

  struct Particle {
    Vector pos;
    float angle;
    };
  std::vector <Particle*> particles;
};

void load_object_gfx();

#endif 

/* Local Variables: */
/* mode:c++ */
/* End: */
