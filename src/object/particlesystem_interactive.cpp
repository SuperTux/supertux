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

#include "particlesystem_interactive.h"
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
#include "object/rainsplash.h"
#include "badguy/bomb.h"

//TODO: Dynamically create splashes at collision spots
//      Find a way to make rain collide with objects like bonus blocks
//      Add an option to set rain strength
ParticleSystem_Interactive::ParticleSystem_Interactive()
{
    virtual_width = SCREEN_WIDTH;
    virtual_height = SCREEN_HEIGHT;
    layer = LAYER_TILES;
}

ParticleSystem_Interactive::~ParticleSystem_Interactive()
{
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        delete *i;
    }
}

void ParticleSystem_Interactive::draw(DrawingContext& context)
{
  context.push_transform();
  
    std::vector<Particle*>::iterator i;
    for(i = particles.begin(); i != particles.end(); ++i) {
        Particle* particle = *i;
        context.draw_surface(particle->texture, particle->pos, layer);
    }

    context.pop_transform();
}

bool
ParticleSystem_Interactive::collision(Particle* object, Vector movement)
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

RainParticleSystem::RainParticleSystem()
{
    rainimages[0] = new Surface(datadir+"/images/objects/particles/rain0.png", true);
    rainimages[1] = new Surface(datadir+"/images/objects/particles/rain1.png", true);

    virtual_width = SCREEN_WIDTH * 2;

    // create some random raindrops
    size_t raindropcount = size_t(virtual_width/6.0);
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
            //Create rainsplash
            if (particle->pos.y <= SCREEN_HEIGHT + abs_y){
              //TODO: Find out at which side of the tile the collision happens
              bool vertical = false;
              int splash_x, splash_y;
              if (vertical) {
                splash_x = int(particle->pos.x) - (int(particle->pos.x) % 32) + 32;
                splash_y = int(particle->pos.y);                
              }
              else {
                splash_x = int(particle->pos.x);
                splash_y = int(particle->pos.y) - (int(particle->pos.y) % 32) + 32;
              }
              Sector::current()->add_object(new RainSplash(Vector(splash_x, splash_y),vertical));
            }
            int new_x = (rand() % int(virtual_width)) + int(abs_x);
            int new_y = 0;
            //FIXME: Don't move particles over solid tiles
            particle->pos.x = new_x;
            particle->pos.y = new_y;
        }
    }
}

CometParticleSystem::CometParticleSystem()
{
    cometimages[0] = new Surface(datadir+"/images/creatures/mr_bomb/exploding-left-0.png", true);
    cometimages[1] = new Surface(datadir+"/images/creatures/mr_bomb/exploding-left-0.png", true);

    virtual_width = SCREEN_WIDTH * 2;

    // create some random comets
    size_t cometcount = 2;
    for(size_t i=0; i<cometcount; ++i) {
        CometParticle* particle = new CometParticle;
        particle->pos.x = rand() % int(virtual_width);
        particle->pos.y = rand() % int(virtual_height);
        int cometsize = rand() % 2;
        particle->texture = cometimages[cometsize];
        do {
            particle->speed = (cometsize+1)*30 + (float(rand()%10)*.4);
        } while(particle->speed < 1);
        particle->speed *= 10; // gravity

        particles.push_back(particle);
    }
}

void
CometParticleSystem::parse(const lisp::Lisp& reader)
{
  reader.get("layer", layer);
}

void
CometParticleSystem::write(lisp::Writer& writer)
{
  writer.start_list("particles-comets");
  writer.write_int("layer", layer);
  writer.end_list("particles-comets");
}

CometParticleSystem::~CometParticleSystem()
{
  for(int i=0;i<2;++i)
    delete cometimages[i];
}

void CometParticleSystem::update(float elapsed_time)
{
    std::vector<Particle*>::iterator i;
    for(
        i = particles.begin(); i != particles.end(); ++i) {
        CometParticle* particle = (CometParticle*) *i;
        float movement = particle->speed * elapsed_time;
        float abs_x = Sector::current()->camera->get_translation().x;
        float abs_y = Sector::current()->camera->get_translation().y;
        particle->pos.y += movement;
        particle->pos.x -= movement;
        if ((particle->pos.y > SCREEN_HEIGHT + abs_y) || (collision(particle, Vector(-movement, movement)))) {
            if (particle->pos.y <= SCREEN_HEIGHT + abs_y) {
              Sector::current()->add_object(new Bomb(particle->pos, LEFT));
            }
            int new_x = (rand() % int(virtual_width)) + int(abs_x);
            int new_y = 0;
            //FIXME: Don't move particles over solid tiles
            particle->pos.x = new_x;
            particle->pos.y = new_y;
        }
    }
}
