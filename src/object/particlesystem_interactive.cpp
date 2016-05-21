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

#include "object/particlesystem_interactive.hpp"

#include "math/aatriangle.hpp"
#include "object/tilemap.hpp"
#include "supertux/collision.hpp"
#include "supertux/tile.hpp"

//TODO: Find a way to make rain collide with objects like bonus blocks
//      Add an option to set rain strength
//      Fix rain being "respawned" over solid tiles
ParticleSystem_Interactive::ParticleSystem_Interactive() :
  z_pos(),
  particles(),
  virtual_width(),
  virtual_height()
{
  virtual_width = SCREEN_WIDTH;
  virtual_height = SCREEN_HEIGHT;
  z_pos = 0;
}

ParticleSystem_Interactive::~ParticleSystem_Interactive()
{
  for(auto& particle : particles) {
    delete particle;
  }
}

void ParticleSystem_Interactive::parse(const ReaderMapping& reader)
{
  z_pos = reader_get_layer (reader, /* default = */ LAYER_BACKGROUND1);
}

void ParticleSystem_Interactive::draw(DrawingContext& context)
{
  context.push_transform();

  for(auto& particle : particles) {
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
  if (x2 < x1) {
    x1 = x2;
    x2 = object->pos.x;
  }

  y1 = object->pos.y;
  y2 = y1 + 32 + movement.y;
  if (y2 < y1) {
    y1 = y2;
    y2 = object->pos.y;
  }
  bool water = false;

  // test with all tiles in this rectangle
  int starttilex = int(x1-1) / 32;
  int starttiley = int(y1-1) / 32;
  int max_x = int(x2+1);
  int max_y = int(y2+1);

  Rectf dest(x1, y1, x2, y2);
  dest.move(movement);
  Constraints constraints;

  for(const auto& solids : Sector::current()->solid_tilemaps) {
    // FIXME Handle a nonzero tilemap offset
    for(int x = starttilex; x*32 < max_x; ++x) {
      for(int y = starttiley; y*32 < max_y; ++y) {
        const Tile* tile = solids->get_tile(x, y);
        if(!tile)
          continue;
        // skip non-solid tiles, except water
        if(! (tile->getAttributes() & (Tile::WATER | Tile::SOLID)))
          continue;

        Rectf rect = solids->get_tile_bbox(x, y);
        if(tile->is_slope ()) { // slope tile
          AATriangle triangle = AATriangle(rect, tile->getData());

          if(rectangle_aatriangle(&constraints, dest, triangle)) {
            if(tile->getAttributes() & Tile::WATER)
              water = true;
          }
        } else { // normal rectangular tile
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

ObjectSettings
ParticleSystem_Interactive::get_settings() {
  ObjectSettings result = GameObject::get_settings();
  result.options.push_back( ObjectOption(MN_INTFIELD, _("Z-pos"), &z_pos));

  result.options.push_back( ObjectOption(MN_REMOVE, "", NULL));
  return result;
}


/* EOF */
