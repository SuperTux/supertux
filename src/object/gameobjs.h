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

#include "video/surface.h"
#include "timer.h"
#include "math/physic.h"
#include "special/game_object.h"
#include "special/moving_object.h"
#include "serializable.h"

/* Bounciness of distros: */
#define NO_BOUNCE 0
#define BOUNCE 1

namespace SuperTux {
class Sprite;
}

class BouncyCoin : public GameObject
{
public:
  BouncyCoin(const Vector& pos);
  ~BouncyCoin();
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Sprite* sprite;
  Vector position;
  Timer2 timer;
};

class BrokenBrick : public GameObject
{
public:
  BrokenBrick(Sprite* sprite, const Vector& pos, const Vector& movement);
  ~BrokenBrick();

  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Timer2 timer;
  Sprite* sprite;
  Vector position;
  Vector movement;
};

class FloatingText : public GameObject
{
public:
  FloatingText(const Vector& pos, const std::string& text_);
  FloatingText(const Vector& pos, int s);  // use this for score, for instance
  
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Vector position;
  std::string text;
  Timer2 timer;  
};

extern Sprite *img_smoke_cloud;

class SmokeCloud : public GameObject
{
public:
  SmokeCloud(const Vector& pos);
  
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Timer2 timer;
  Vector position;
};

class Particles : public GameObject
{
public:
  Particles(const Vector& epicenter, int min_angle, int max_angle,
            const Vector& initial_velocity, const Vector& acceleration,
            int number, Color color, int size, float life_time, int drawing_layer);
  ~Particles();
  
  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Vector accel;
  Timer2 timer;
  bool live_forever;

  Color color;
  float size;
  int drawing_layer;

  struct Particle {
    Vector pos, vel;
//     float angle;
    };
  std::vector <Particle*> particles;
};

void load_object_gfx();
void free_object_gfx();

#endif 

/* Local Variables: */
/* mode:c++ */
/* End: */
