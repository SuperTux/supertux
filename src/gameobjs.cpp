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

#include <algorithm>
#include <iostream>
#include <cmath>

#include "app/globals.h"
#include "tile.h"
#include "tile_manager.h"
#include "gameloop.h"
#include "gameobjs.h"
#include "special/sprite_manager.h"
#include "resources.h"
#include "sector.h"
#include "tilemap.h"
#include "video/drawing_context.h"
#include "camera.h"

BouncyDistro::BouncyDistro(const Vector& pos)
  : position(pos)
{
  ym = -2;
}

void
BouncyDistro::action(float elapsed_time)
{
  position.y += ym * elapsed_time;

  ym += 0.1 * elapsed_time; // not framerate independent... but who really cares
  if(ym >= 0)
    remove_me();
}

void
BouncyDistro::draw(DrawingContext& context)
{
  context.draw_surface(img_distro[0], position, LAYER_OBJECTS);
}


BrokenBrick::BrokenBrick(Tile* ntile,const Vector& pos, const Vector& nmovement)
  : tile(ntile), position(pos), movement(nmovement)
{
  timer.start(200);
}

void
BrokenBrick::action(float elapsed_time)
{
  position += movement * elapsed_time;

  if (!timer.check())
    remove_me();
}

void
BrokenBrick::draw(DrawingContext& context)
{
  if (tile->images.size() > 0)
    context.draw_surface_part(tile->images[0],
        Vector(rand() % 16, rand() % 16),
        Vector(16, 16),
        position, LAYER_OBJECTS + 1);
}

BouncyBrick::BouncyBrick(const Vector& pos)
  : position(pos), offset(0), offset_m(-BOUNCY_BRICK_SPEED), 
    shape(Sector::current()->solids->get_tile_id_at(pos))
{ 
  shape.hidden = true;
}

void
BouncyBrick::action(float elapsed_time)
{
  offset += offset_m * elapsed_time;

  /* Go back down? */
  if (offset < -BOUNCY_BRICK_MAX_OFFSET)
    offset_m = BOUNCY_BRICK_SPEED;

  /* Stop bouncing? */
  if (offset >= 0)
    {
      shape.hidden = false;
      remove_me();
    }
}

void
BouncyBrick::draw(DrawingContext& context)
{
  TileManager::instance()->
    draw_tile(context, shape.id, position + Vector(0, offset), LAYER_TILES+1);
}

FloatingText::FloatingText(const Vector& pos, const std::string& text_)
  : position(pos), text(text_)
{
  timer.start(1000);
  position.x -= text.size() * 8;
}

FloatingText::FloatingText(const Vector& pos, int score)
  : position(pos)
{
  timer.start(1000);

  // turn int into a string
  char str[10];
  snprintf(str, 10, "%d", score);
  text = str;

  position.x -= text.size() * 8;
}

void
FloatingText::action(float elapsed_time)
{
  position.y -= 1.4 * elapsed_time;

  if(!timer.check())
    remove_me();
}

#define FADING_TIME 350

void
FloatingText::draw(DrawingContext& context)
{
  // make an alpha animation when disapearing
  int alpha;
  if(timer.get_left() < FADING_TIME)
    alpha = timer.get_left() * 255 / FADING_TIME;
  else
    alpha = 255;

  context.draw_text(gold_text, text, position, LEFT_ALLIGN, LAYER_OBJECTS, NONE_EFFECT, alpha);
}

/* Trampoline */

Sprite *img_trampoline;

Trampoline::Trampoline(LispReader& reader)
{
  reader.read_float("x", base.x);
  reader.read_float("y", base.y); 
  base.width = 32;
  base.height = 32;
  power = 7.5;
  reader.read_float("power", power);

  frame = 0;
  mode = M_NORMAL;
  physic.reset();
}

Trampoline::Trampoline(float x, float y)
{
  base.x = x;
  base.y = y;
  base.width = 32;
  base.height = 32;
  power = 7.5;

  frame = 0;
  mode = M_NORMAL;
  physic.reset();
}

void
Trampoline::write(LispWriter& writer)
{
  writer.start_list("trampoline");

  writer.write_float("x", base.x);
  writer.write_float("y", base.y);
  writer.write_float("power", power);

  writer.end_list("trampoline");
}

void
Trampoline::draw(DrawingContext& context)
{
  img_trampoline->set_frame(frame);
  img_trampoline->draw(context, base, LAYER_OBJECTS);
  frame = 0;
}

void
Trampoline::action(float frame_ratio)
{
  // TODO: Remove if we're too far off the screen

  // Falling
  if (mode != M_HELD)
  {
    if (issolid(base.x + base.width/2, base.y + base.height))
    {
      base.y = int((base.y + base.height)/32) * 32 - base.height;

      physic.enable_gravity(false);
      physic.set_velocity_y(0.0f);

      physic.set_velocity_x(0);
    }
    else
    {
      physic.enable_gravity(true);
    }
  }
  else // Player is carrying us around
  {
    /* FIXME: The trampoline object shouldn't know about pplayer objects. */
    /* If we're holding the iceblock */
    Player& tux = *Sector::current()->player;
    Direction dir = tux.dir;

    if(dir == RIGHT)
    {
      base.x = tux.base.x + 16;
      base.y = tux.base.y + tux.base.height/1.5 - base.height;
    }
    else /* facing left */
    {
      base.x = tux.base.x - 16;
      base.y = tux.base.y + tux.base.height/1.5 - base.height;
    }

    if(collision_object_map(base))
    {
      base.x = tux.base.x;
      base.y = tux.base.y + tux.base.height/1.5 - base.height;
    }
  }

  physic.apply(frame_ratio, base.x, base.y, Sector::current()->gravity);
  collision_swept_object_map(&old_base, &base);
}

void
Trampoline::collision(const MovingObject&, int)
{
  // comes later
}

void
Trampoline::collision(void *p_c_object, int c_object, CollisionType type)
{
  Player* pplayer_c = NULL;
  switch (c_object)
  {
    case CO_PLAYER:
      pplayer_c = (Player*) p_c_object;

      if (type == COLLISION_NORMAL)
      {
        // Pick up if HELD (done in Player)
      }

      else if (type == COLLISION_SQUISH)
      {
        int squish_amount = (32 - (int)pplayer_c->base.y % 32);

        if (squish_amount < 24)
          frame = 3;
        else if (squish_amount < 28)
          frame = 2;
        else if (squish_amount < 30)
          frame = 1;
        else
          frame = 0;

        if (squish_amount < 20) {
          pplayer_c->physic.set_velocity_y(power);
          pplayer_c->fall_mode = Player::TRAMPOLINE_JUMP;
        }
        else if (pplayer_c->physic.get_velocity_y() < 0)
          pplayer_c->physic.set_velocity_y(-squish_amount/32);
      }

      break;

    default:
      break;
    
  }
}

/* Flying Platform */

Sprite *img_flying_platform;

FlyingPlatform::FlyingPlatform(LispReader& reader)
{
  reader.read_int_vector("x", pos_x);
  reader.read_int_vector("y", pos_y);

  velocity = 2.0;
  reader.read_float("velocity", velocity);

  base.x = pos_x[0];
  base.y = pos_y[0];
  base.width = 96;
  base.height = 40;

  point = 0;
  move = false;

  float x = pos_x[point+1] - pos_x[point];
  float y = pos_y[point+1] - pos_y[point];
  vel_x = x*velocity / sqrt(x*x + y*y);
  vel_y = -(velocity - vel_x);

  frame = 0;
}

FlyingPlatform::FlyingPlatform(int x, int y)
{
base.x = x;
base.y = y;
point = 0;
move = false;
}

void
FlyingPlatform::write(LispWriter& writer)
{
  writer.start_list("flying-trampoline");

  writer.write_int_vector("x", pos_x);
  writer.write_int_vector("y", pos_y);
  writer.write_float("velocity", velocity);

  writer.end_list("flying-trampoline");
}

void
FlyingPlatform::draw(DrawingContext& context)
{
  img_flying_platform->draw(context, base, LAYER_OBJECTS);
}

void
FlyingPlatform::action(float frame_ratio)
{
  // TODO: Remove if we're too far off the screen

if(!move)
  return;

if((unsigned)point+1 != pos_x.size())
  {
  if(((pos_x[point+1] > pos_x[point] && base.x >= pos_x[point+1]) ||
      (pos_x[point+1] < pos_x[point] && base.x <= pos_x[point+1]) ||
      pos_x[point] == pos_x[point+1]) &&
    ((pos_y[point+1] > pos_y[point] && base.y >= pos_y[point+1]) ||
      (pos_y[point+1] < pos_y[point] && base.y <= pos_y[point+1]) ||
      pos_y[point] == pos_y[point+1]))
    {
    point++;

    float x = pos_x[point+1] - pos_x[point];
    float y = pos_y[point+1] - pos_y[point];
    vel_x = x*velocity / sqrt(x*x + y*y);
    vel_y = -(velocity - vel_x);
    }
  }
else   // last point
  {
  // point = 0;
  // reverse vector
  return;
  }
/*
if(pos_x[point+1] > base.x)
  base.x += velocity * frame_ratio;
else if(pos_x[point+1] < base.x)
  base.x -= velocity * frame_ratio;

if(pos_y[point+1] > base.y)
  base.y += velocity * frame_ratio;
else if(pos_y[point+1] < base.y)
  base.y -= velocity * frame_ratio;
*/

base.x += vel_x * frame_ratio;
base.y += vel_y * frame_ratio;
}

void
FlyingPlatform::collision(const MovingObject&, int)
{
  // comes later
}

void
FlyingPlatform::collision(void *p_c_object, int c_object, CollisionType type)
{
(void) p_c_object;
(void) type;

//  Player* pplayer_c = NULL;
  switch (c_object)
  {
    case CO_PLAYER:
//      pplayer_c = (Player*) p_c_object;
      move = true;

      break;

    default:
      break;
    
  }
}

Sprite *img_smoke_cloud;

SmokeCloud::SmokeCloud(const Vector& pos)
  : position(pos)
{
  timer.start(300);
}

void
SmokeCloud::action(float elapsed_time)
{
  position.y -= 1.2 * elapsed_time;

  if(!timer.check())
    remove_me();
}

void
SmokeCloud::draw(DrawingContext& context)
{
  img_smoke_cloud->draw(context, position, LAYER_OBJECTS+1);
}

Particles::Particles(const Vector& epicenter, const Vector& velocity, const Vector& acceleration, int number, Color color_, int size_, int life_time)
  : color(color_), size(size_), vel(velocity), accel(acceleration)
{
  if(life_time == 0)
    {
    live_forever = true;
    }
  else
    {
    live_forever = false;
    timer.start(life_time);
    }

  // create particles
  for(int p = 0; p < number; p++)
    {
    Particle* particle = new Particle;
    particle->pos = epicenter;
    particle->angle = (rand() % 360) * (M_PI / 180);  // in radius

    particles.push_back(particle);
    }
}

Particles::~Particles()
{
  // free particles
  for(std::vector<Particle*>::iterator i = particles.begin(); i < particles.end(); i++)
    delete (*i);
}

void
Particles::action(float elapsed_time)
{
  vel.x += accel.x * elapsed_time;
  vel.y += accel.y * elapsed_time;

  int camera_x = (int)Sector::current()->camera->get_translation().x;
  int camera_y = (int)Sector::current()->camera->get_translation().y;

  // update particles
  for(std::vector<Particle*>::iterator i = particles.begin(); i < particles.end(); i++)
    {
    (*i)->pos.x += sin((*i)->angle) * vel.x * elapsed_time;
    (*i)->pos.y += cos((*i)->angle) * vel.y * elapsed_time;

    if((*i)->pos.x < camera_x || (*i)->pos.x > screen->w + camera_x ||
       (*i)->pos.y < camera_y || (*i)->pos.y > screen->h + camera_y)
      {
      delete (*i);
      particles.erase(i);
      }
    }

  if((!timer.check() && !live_forever) || particles.size() == 0)
    remove_me();
}

void
Particles::draw(DrawingContext& context)
{
  // draw particles
  for(std::vector<Particle*>::iterator i = particles.begin(); i < particles.end(); i++)
    {
    context.draw_filled_rect((*i)->pos, Vector(size,size), color, LAYER_OBJECTS+10);
    }
}

void load_object_gfx()
{
  img_trampoline = sprite_manager->load("trampoline");
  img_trampoline->start_animation(0);
  img_flying_platform = sprite_manager->load("flying_platform");
  img_smoke_cloud = sprite_manager->load("stomp");
}
