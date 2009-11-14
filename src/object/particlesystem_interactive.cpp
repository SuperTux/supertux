//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <config.h>

#include <iostream>
#include <cmath>

#include "particlesystem_interactive.hpp"
#include "video/drawing_context.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"
#include "resources.hpp"
#include "main.hpp"

#include "tile.hpp"
#include "tilemap.hpp"
#include "math/aatriangle.hpp"
#include "collision.hpp"
#include "collision_hit.hpp"
#include "object/camera.hpp"
#include "object/rainsplash.hpp"
#include "badguy/bomb.hpp"
#include "math/random_generator.hpp"

//TODO: Find a way to make rain collide with objects like bonus blocks
//      Add an option to set rain strength
//      Fix rain being "respawned" over solid tiles
ParticleSystem_Interactive::ParticleSystem_Interactive()
{
    virtual_width = SCREEN_WIDTH;
    virtual_height = SCREEN_HEIGHT;
    z_pos = 0;
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
        context.draw_surface(particle->texture, particle->pos, z_pos);
    }

    context.pop_transform();
}

int
ParticleSystem_Interactive::collision(Particle* object, Vector movement)
{
  using namespace collision;

  // calculate rectangle where the object will move
  float x1, x2;
  float y1, y2;
  x1 = object->pos.x;
  x2 = x1 + 32 + movement.x;
  y1 = object->pos.y;
  y2 = y1 + 32 + movement.y;
  bool water = false;

  // test with all tiles in this rectangle
  int starttilex = int(x1-1) / 32;
  int starttiley = int(y1-1) / 32;
  int max_x = int(x2+1);
  int max_y = int(y2+1);

  Rect dest = Rect(x1, y1, x2, y2);
  dest.move(movement);
  Constraints constraints;

  for(std::list<TileMap*>::const_iterator i = Sector::current()->solid_tilemaps.begin(); i != Sector::current()->solid_tilemaps.end(); i++) {
    TileMap* solids = *i;
    for(int x = starttilex; x*32 < max_x; ++x) {
      for(int y = starttiley; y*32 < max_y; ++y) {
        const Tile* tile = solids->get_tile(x, y);
        if(!tile)
          continue;
        // skip non-solid tiles, except water
        if(! (tile->getAttributes() & (Tile::WATER | Tile::SOLID)))
          continue;

        if(tile->getAttributes() & Tile::SLOPE) { // slope tile
          AATriangle triangle;
          Vector p1(x*32, y*32);
          Vector p2((x+1)*32, (y+1)*32);
          triangle = AATriangle(p1, p2, tile->getData());

          if(rectangle_aatriangle(&constraints, dest, triangle)) {
            if(tile->getAttributes() & Tile::WATER)
              water = true;
          }
        } else { // normal rectangular tile
          Rect rect(x*32, y*32, (x+1)*32, (y+1)*32);
          if(intersects(dest, rect)) {
            if(tile->getAttributes() & Tile::WATER)
              water = true;
            set_rectangle_rectangle_constraints(&constraints, dest, rect);
          }
        }
      }
    }
  }


  // TODO don't use magic numbers here...

  // did we collide at all?
  if(!constraints.has_constraints())
    return -1;

  const CollisionHit& hit = constraints.hit;
  if (water) {
    return 0; //collision with water tile - don't draw splash
  } else {
    if (hit.right || hit.left) {
      return 2; //collision from right
    } else {
      return 1; //collision from above
    }
  }

  return 0;
}

RainParticleSystem::RainParticleSystem()
{
    rainimages[0] = new Surface("images/objects/particles/rain0.png");
    rainimages[1] = new Surface("images/objects/particles/rain1.png");

    virtual_width = SCREEN_WIDTH * 2;

    // create some random raindrops
    size_t raindropcount = size_t(virtual_width/6.0);
    for(size_t i=0; i<raindropcount; ++i) {
        RainParticle* particle = new RainParticle;
        particle->pos.x = systemRandom.rand(int(virtual_width));
        particle->pos.y = systemRandom.rand(int(virtual_height));
        int rainsize = systemRandom.rand(2);
        particle->texture = rainimages[rainsize];
        do {
            particle->speed = (rainsize+1)*45 + systemRandom.randf(3.6);
        } while(particle->speed < 1);
        particle->speed *= 10; // gravity

        particles.push_back(particle);
    }
}

void
RainParticleSystem::parse(const lisp::Lisp& reader)
{
  reader.get("z-pos", z_pos);
}

void
RainParticleSystem::write(lisp::Writer& writer)
{
  writer.start_list("particles-rain");
  writer.write("z-pos", z_pos);
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
        int col = collision(particle, Vector(-movement, movement));
        if ((particle->pos.y > SCREEN_HEIGHT + abs_y) || (col >= 0)) {
            //Create rainsplash
            if ((particle->pos.y <= SCREEN_HEIGHT + abs_y) && (col >= 1)){
              bool vertical = (col == 2);
              int splash_x, splash_y;
              if (!vertical) { //check if collision happened from above
                splash_x = int(particle->pos.x);
                splash_y = int(particle->pos.y) - (int(particle->pos.y) % 32) + 32;
                Sector::current()->add_object(new RainSplash(Vector(splash_x, splash_y),vertical));
              }
              // Uncomment the following to display vertical splashes, too
              /* else {
                splash_x = int(particle->pos.x) - (int(particle->pos.x) % 32) + 32;
                splash_y = int(particle->pos.y);
                Sector::current()->add_object(new RainSplash(Vector(splash_x, splash_y),vertical));
              } */
            }
            int new_x = systemRandom.rand(int(virtual_width)) + int(abs_x);
            int new_y = 0;
            //FIXME: Don't move particles over solid tiles
            particle->pos.x = new_x;
            particle->pos.y = new_y;
        }
    }
}

CometParticleSystem::CometParticleSystem()
{
    cometimages[0] = new Surface("images/creatures/mr_bomb/exploding-left-0.png");
    cometimages[1] = new Surface("images/creatures/mr_bomb/exploding-left-0.png");

    virtual_width = SCREEN_WIDTH * 2;

    // create some random comets
    size_t cometcount = 2;
    for(size_t i=0; i<cometcount; ++i) {
        CometParticle* particle = new CometParticle;
        particle->pos.x = systemRandom.rand(int(virtual_width));
        particle->pos.y = systemRandom.rand(int(virtual_height));
        int cometsize = systemRandom.rand(2);
        particle->texture = cometimages[cometsize];
        do {
            particle->speed = (cometsize+1)*30 + systemRandom.randf(3.6);
        } while(particle->speed < 1);
        particle->speed *= 10; // gravity

        particles.push_back(particle);
    }
}

void
CometParticleSystem::parse(const lisp::Lisp& reader)
{
  reader.get("z-pos", z_pos);
}

void
CometParticleSystem::write(lisp::Writer& writer)
{
  writer.start_list("particles-comets");
  writer.write("z-pos", z_pos);
  writer.end_list("particles-comets");
}

CometParticleSystem::~CometParticleSystem()
{
  for(int i=0;i<2;++i)
    delete cometimages[i];
}

void CometParticleSystem::update(float elapsed_time)
{
  (void) elapsed_time;
#if 0
    std::vector<Particle*>::iterator i;
    for(
        i = particles.begin(); i != particles.end(); ++i) {
        CometParticle* particle = (CometParticle*) *i;
        float movement = particle->speed * elapsed_time;
        float abs_x = Sector::current()->camera->get_translation().x;
        float abs_y = Sector::current()->camera->get_translation().y;
        particle->pos.y += movement;
        particle->pos.x -= movement;
        int col = collision(particle, Vector(-movement, movement));
        if ((particle->pos.y > SCREEN_HEIGHT + abs_y) || (col >= 0)) {
            if ((particle->pos.y <= SCREEN_HEIGHT + abs_y) && (col >= 1)) {
              Sector::current()->add_object(new Bomb(particle->pos, LEFT));
            }
            int new_x = systemRandom.rand(int(virtual_width)) + int(abs_x);
            int new_y = 0;
            //FIXME: Don't move particles over solid tiles
            particle->pos.x = new_x;
            particle->pos.y = new_y;
        }
    }
#endif
}
