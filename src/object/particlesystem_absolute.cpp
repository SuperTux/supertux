//  $Id: particlesystem.cpp 2470 2005-05-11 14:49:28Z wansti $
// 
//  SuperTux
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de>
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
#include <config.h>

#include <iostream>
#include <cmath>

#include "particlesystem_absolute.h"
#include "video/drawing_context.h"
#include "lisp/parser.h"
#include "lisp/lisp.h"
#include "lisp/writer.h"
#include "resources.h"
#include "main.h"

#include "tile.h"
#include "tilemap.h"
#include "math/aatriangle.h"
#include "collision.h"
#include "collision_hit.h"
#include "object/camera.h"

//TODO: Try to equally distribute rain throughout the level
//      Dynamically create splashes at collision spots
//      Check why it can rain through boxes
//      Add an option to set rain strength
ParticleSystem_Absolute::ParticleSystem_Absolute()
{
    virtual_width = SCREEN_WIDTH;
    virtual_height = SCREEN_HEIGHT;
    layer = LAYER_BACKGROUND1;
}

ParticleSystem_Absolute::~ParticleSystem_Absolute()
{
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        delete *i;
    }
}

void ParticleSystem_Absolute::draw(DrawingContext& context)
{
  //float scrollx = context.get_translation().x;
  //float scrolly = context.get_translation().y;

  context.push_transform();
  //context.set_translation(Vector(0,0));
  
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        Particle* particle = *i;

        // remap x,y coordinates onto screencoordinates
        /*Vector pos;
        pos.x = fmodf(particle->pos.x - scrollx, virtual_width);
        if(pos.x < 0) pos.x += virtual_width;
        pos.y = fmodf(particle->pos.y - scrolly, virtual_height);
        if(pos.y < 0) pos.y += virtual_height;

        if(pos.x > SCREEN_WIDTH) pos.x -= virtual_width;
        if(pos.y > SCREEN_HEIGHT) pos.y -= virtual_height;*/
        context.draw_surface(particle->texture, particle->pos, layer);
    }

    context.pop_transform();
}

RainParticleSystem::RainParticleSystem()
{
    rainimages[0] = new Surface(datadir+"/images/objects/particles/rain0.png", true);
    rainimages[1] = new Surface(datadir+"/images/objects/particles/rain1.png", true);

    virtual_width = SCREEN_WIDTH * 2;

    // create some random raindrops
    size_t raindropcount = size_t(virtual_width/4.0);
    for(size_t i=0; i<raindropcount; ++i) {
        RainParticle* particle = new RainParticle;
        particle->pos.x = rand() % int(virtual_width);
        particle->pos.y = rand() % int(virtual_height);
        int rainsize = rand() % 2;
        particle->texture = rainimages[rainsize];
        do {
            particle->speed = (rainsize+1)*45 + (float(rand()%10)*.4);
        } while(particle->speed < 1);
        particle->speed *= 10; // gravity

        particles.push_back(particle);
    }
}

void
RainParticleSystem::parse(const lisp::Lisp& reader)
{
  reader.get("layer", layer);
}

void
RainParticleSystem::write(lisp::Writer& writer)
{
  writer.start_list("particles-rain");
  writer.write_int("layer", layer);
  writer.end_list("particles-rain");
}

RainParticleSystem::~RainParticleSystem()
{
  for(int i=0;i<2;++i)
    delete rainimages[i];
}

void RainParticleSystem::update(float elapsed_time)
{
    std::vector<Particle*>::iterator i;
    for(
        i = particles.begin(); i != particles.end(); ++i) {
        RainParticle* particle = (RainParticle*) *i;
        float movement = particle->speed * elapsed_time;
        float abs_x = Sector::current()->camera->get_translation().x;
        float abs_y = Sector::current()->camera->get_translation().y;
        particle->pos.y += movement;
        particle->pos.x -= movement;
        if ((particle->pos.y > SCREEN_HEIGHT + abs_y) || (collision(particle, Vector(-movement, movement)))) {
            particle->pos.y = 0;
            particle->pos.x = rand() % int(abs_x + virtual_width);
        }
    }
}

bool
RainParticleSystem::collision(RainParticle* object, Vector movement)
{
  TileMap* solids = Sector::current()->solids;
  // calculate rectangle where the object will move
  float x1, x2;
  float y1, y2;
  x1 = object->pos.x;
  x2 = x1 + 32 + movement.x;
  y1 = object->pos.y;
  y2 = y1 + 32 + movement.y;
  
  // test with all tiles in this rectangle
  int starttilex = int(x1-1) / 32;
  int starttiley = int(y1-1) / 32;
  int max_x = int(x2+1);
  int max_y = int(y2+1);
  
  CollisionHit temphit, hit;
  Rect dest = Rect(x1, y1, x2, y2);
  dest.move(movement);
  hit.time = -1; // represents an invalid value
  for(int x = starttilex; x*32 < max_x; ++x) {
    for(int y = starttiley; y*32 < max_y; ++y) {
      const Tile* tile = solids->get_tile(x, y);
      //std::cout << "Tile at: " << x << "," << y << std::endl;
      if(!tile)
        continue;
      // skip non-solid tiles
      if(!(tile->getAttributes() & Tile::SOLID))
        continue;

      if(tile->getAttributes() & Tile::SLOPE) { // slope tile
        AATriangle triangle;
        Vector p1(x*32, y*32);
        Vector p2((x+1)*32, (y+1)*32);
        triangle = AATriangle(p1, p2, tile->getData());

        if(Collision::rectangle_aatriangle(temphit, dest, movement,
              triangle)) {
          if(temphit.time > hit.time)
            hit = temphit;
        }
      } else { // normal rectangular tile
        Rect rect(x*32, y*32, (x+1)*32, (y+1)*32);
        if(Collision::rectangle_rectangle(temphit, dest,
              movement, rect)) {
          if(temphit.time > hit.time)
            hit = temphit;
        }
      }
    }
  }
  
  // did we collide at all?
  if(hit.time < 0)
    return false; else return true;
}
