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
#include "world.h"
#include "tile.h"
#include "gameloop.h"
#include "gameobjs.h"
#include "sprite_manager.h"
#include "resources.h"
#include "level.h"
#include "display_manager.h"

BouncyDistro::BouncyDistro(DisplayManager& displaymanager, const Vector& pos)
  : position(pos)
{
  ym = -2;
  displaymanager.add_drawable(this, LAYER_OBJECTS);
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
BouncyDistro::draw(Camera& viewport, int )
{
  img_distro[0]->draw(viewport.world2screen(position));
}


BrokenBrick::BrokenBrick(DisplayManager& displaymanager, Tile* ntile,
    const Vector& pos, const Vector& nmovement)
  : tile(ntile), position(pos), movement(nmovement)
{
  displaymanager.add_drawable(this, LAYER_OBJECTS);
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
BrokenBrick::draw(Camera& viewport, int )
{
  SDL_Rect src, dest;
  src.x = rand() % 16;
  src.y = rand() % 16;
  src.w = 16;
  src.h = 16;

  dest.x = (int)(position.x - viewport.get_translation().x);
  dest.y = (int)(position.y - viewport.get_translation().y);
  dest.w = 16;
  dest.h = 16;
  
  if (tile->images.size() > 0)
    tile->images[0]->draw_part(src.x,src.y,dest.x,dest.y,dest.w,dest.h);
}

BouncyBrick::BouncyBrick(DisplayManager& displaymanager, const Vector& pos)
  : position(pos), offset(0), offset_m(-BOUNCY_BRICK_SPEED)
{
  displaymanager.add_drawable(this, LAYER_OBJECTS);
  shape    = World::current()->get_level()->gettileid(pos.x, pos.y);
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
    remove_me();
}

void
BouncyBrick::draw(Camera& viewport, int)
{
  Tile::draw(viewport.world2screen(position + Vector(0, offset)), shape);
}

FloatingScore::FloatingScore(DisplayManager& displaymanager, 
    const Vector& pos, int score)
  : position(pos)
{
  displaymanager.add_drawable(this, LAYER_OBJECTS);
  timer.start(1000);
  snprintf(str, 10, "%d", score);
  position.x -= strlen(str) * 8;
}

void
FloatingScore::action(float elapsed_time)
{
  position.y -= 2 * elapsed_time;

  if(!timer.check())
    remove_me();
}

void
FloatingScore::draw(Camera& viewport, int )
{
  gold_text->draw(str, viewport.world2screen(position));
}

/* Trampoline */

#define TRAMPOLINE_FRAMES 4
Sprite *img_trampoline[TRAMPOLINE_FRAMES];

Trampoline::Trampoline(DisplayManager& displaymanager, LispReader& reader)
{
  displaymanager.add_drawable(this, LAYER_OBJECTS);
  
  reader.read_float("x", &base.x);
  reader.read_float("y", &base.y); 
  base.width = 32;
  base.height = 32;
  power = 7.5;
  reader.read_float("power", &power);

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
Trampoline::draw(Camera& viewport, int )
{
  img_trampoline[frame]->draw(viewport.world2screen(Vector(base.x, base.y)));
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
    Player& tux = *World::current()->get_tux();
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

  physic.apply(frame_ratio, base.x, base.y);
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

FlyingPlatform::FlyingPlatform(DisplayManager& displaymanager, LispReader& reader)
{
  displaymanager.add_drawable(this, LAYER_OBJECTS);

  reader.read_int_vector("x",  &pos_x);
  reader.read_int_vector("y",  &pos_y);

  velocity = 2.0;
  reader.read_float("velocity", &velocity);

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
FlyingPlatform::draw(Camera& viewport, int )
{
img_flying_platform->draw(viewport.world2screen(Vector(base.x, base.y)));
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

void load_object_gfx()
{
  char sprite_name[16];

  for (int i = 0; i < TRAMPOLINE_FRAMES; i++)
  {
    sprintf(sprite_name, "trampoline-%i", i+1);
    img_trampoline[i] = sprite_manager->load(sprite_name);
  }

  img_flying_platform = sprite_manager->load("flying_platform");
}
