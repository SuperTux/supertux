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
#include <config.h>

#include <algorithm>
#include <iostream>
#include <cmath>

#include "app/globals.h"
#include "tile.h"
#include "tile_manager.h"
#include "game_session.h"
#include "gameobjs.h"
#include "sprite/sprite_manager.h"
#include "resources.h"
#include "sector.h"
#include "tilemap.h"
#include "video/drawing_context.h"
#include "camera.h"

BouncyCoin::BouncyCoin(const Vector& pos)
  : position(pos)
{
  timer.start(.3);
  sprite = sprite_manager->create("coin");
  sprite->set_action("still");
}

BouncyCoin::~BouncyCoin()
{
  delete sprite;
}

void
BouncyCoin::action(float elapsed_time)
{
  position.y += -200 * elapsed_time;

  if(timer.check())
    remove_me();
}

void
BouncyCoin::draw(DrawingContext& context)
{
  sprite->draw(context, position, LAYER_OBJECTS);
}

//---------------------------------------------------------------------------

BrokenBrick::BrokenBrick(Sprite* nsprite,
    const Vector& pos, const Vector& nmovement)
  : sprite(new Sprite(*nsprite)), position(pos), movement(nmovement)
{
  timer.start(.2);
}

BrokenBrick::~BrokenBrick()
{
  delete sprite;
}

void
BrokenBrick::action(float elapsed_time)
{
  position += movement * elapsed_time;

  if (timer.check())
    remove_me();
}

void
BrokenBrick::draw(DrawingContext& context)
{
  sprite->draw_part(context,
      Vector(rand() % 16, rand() % 16), Vector(16, 16),
      position, LAYER_OBJECTS + 1);
}

//---------------------------------------------------------------------------

FloatingText::FloatingText(const Vector& pos, const std::string& text_)
  : position(pos), text(text_)
{
  timer.start(.1);
  position.x -= text.size() * 8;
}

FloatingText::FloatingText(const Vector& pos, int score)
  : position(pos)
{
  timer.start(.1);

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

  if(timer.check())
    remove_me();
}

#define FADING_TIME .350

void
FloatingText::draw(DrawingContext& context)
{
  // make an alpha animation when disapearing
  int alpha;
  if(timer.get_timeleft() < FADING_TIME)
    alpha = int(timer.get_timeleft() * 255 / FADING_TIME);
  else
    alpha = 255;

  context.push_transform();
  context.set_alpha(alpha);

  context.draw_text(gold_text, text, position, LEFT_ALLIGN, LAYER_OBJECTS+1);

  context.pop_transform();
}

Sprite *img_smoke_cloud = 0;

SmokeCloud::SmokeCloud(const Vector& pos)
  : position(pos)
{
  timer.start(.3);
}

void
SmokeCloud::action(float elapsed_time)
{
  position.y -= 120 * elapsed_time;

  if(timer.check())
    remove_me();
}

void
SmokeCloud::draw(DrawingContext& context)
{
  img_smoke_cloud->draw(context, position, LAYER_OBJECTS+1);
}

Particles::Particles(const Vector& epicenter, int min_angle, int max_angle,
        const Vector& initial_velocity, const Vector& acceleration, int number,
        Color color_, int size_, float life_time, int drawing_layer_)
  : accel(acceleration), color(color_), size(size_), drawing_layer(drawing_layer_)
{
  if(life_time == 0) {
    live_forever = true;
  } else {
    live_forever = false;
    timer.start(life_time);
  }

  // create particles
  for(int p = 0; p < number; p++)
    {
    Particle* particle = new Particle;
    particle->pos = epicenter;

    float angle = ((rand() % (max_angle-min_angle))+min_angle)
                      * (M_PI / 180);  // convert to radius
    particle->vel.x = /*fabs*/(sin(angle)) * initial_velocity.x;
//    if(angle >= M_PI && angle < M_PI*2)
//      particle->vel.x *= -1;  // work around to fix signal
    particle->vel.y = /*fabs*/(cos(angle)) * initial_velocity.y;
//    if(angle >= M_PI_2 && angle < 3*M_PI_2)
//      particle->vel.y *= -1;

    particles.push_back(particle);
    }
}

Particles::~Particles()
{
  // free particles
  for(std::vector<Particle*>::iterator i = particles.begin();
      i < particles.end(); i++)
    delete (*i);
}

void
Particles::action(float elapsed_time)
{
  Vector camera = Sector::current()->camera->get_translation();

  // update particles
  for(std::vector<Particle*>::iterator i = particles.begin();
      i != particles.end(); ) {
    (*i)->pos.x += (*i)->vel.x * elapsed_time;
    (*i)->pos.y += (*i)->vel.y * elapsed_time;

    (*i)->vel.x += accel.x * elapsed_time;
    (*i)->vel.y += accel.y * elapsed_time;

    if((*i)->pos.x < camera.x || (*i)->pos.x > SCREEN_WIDTH + camera.x ||
       (*i)->pos.y < camera.y || (*i)->pos.y > SCREEN_HEIGHT + camera.y) {
      delete (*i);
      i = particles.erase(i);
    } else {
      ++i;
    }
  }

  if((timer.check() && !live_forever) || particles.size() == 0)
    remove_me();
}

void
Particles::draw(DrawingContext& context)
{
  // draw particles
  for(std::vector<Particle*>::iterator i = particles.begin();
      i != particles.end(); i++) {
    context.draw_filled_rect((*i)->pos, Vector(size,size), color,drawing_layer);
  }
}

void load_object_gfx()
{
#if 0
  img_trampoline = sprite_manager->load("trampoline");
  img_trampoline->start_animation(0);
  img_flying_platform = sprite_manager->load("flying_platform");
#endif
  img_smoke_cloud = sprite_manager->create("stomp");
}

void free_object_gfx()
{
  delete img_smoke_cloud;
}

