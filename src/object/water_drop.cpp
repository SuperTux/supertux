//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#ifdef PANDORA
#define _GLIBCXX_USE_C99 1
#endif

#include "object/water_drop.hpp"

#include "audio/sound_manager.hpp"
#include "math/random_generator.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"

WaterDrop::WaterDrop(const Vector& pos, const std::string& sprite_path_, const Vector& velocity) :
  MovingSprite(pos, sprite_path_, LAYER_OBJECTS - 1, COLGROUP_MOVING_ONLY_STATIC),
  physic(),
  wd_state(WDS_FALLING),
  sprite_path(sprite_path_)
{
  physic.enable_gravity(true);
  physic.set_velocity(velocity);
}

void
WaterDrop::update(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time);

  if ( sprite->animation_done() ) {
    remove_me();
  }
}

void
WaterDrop::collision_solid(const CollisionHit& hit)
{
  if(hit.bottom && wd_state == WDS_FALLING) {
    wd_state = WDS_SPLASH;
    physic.enable_gravity(false);
    SoundManager::current()->play("sounds/splash.ogg", get_pos());
    sprite->set_action("splash", 1);

    // spawn water particles
    for (int i = 50; i; i--) {
      int pa = graphicsRandom.rand(0,3);
      float px = graphicsRandom.randf(bbox.p1.x, bbox.p2.x);
      float py = graphicsRandom.randf(bbox.p1.y, bbox.p2.y);
      Vector ppos = Vector(px, py);
      Vector pspeed = ppos - bbox.get_middle();
      pspeed.x *= 12;
      pspeed.y *= 12;
      Sector::current()->add_object(std::make_shared<SpriteParticle>(sprite_path, "particle_" + std::to_string(pa),
                                                                     ppos, ANCHOR_MIDDLE,
                                                                     pspeed, Vector(0, 100 * Sector::current()->get_gravity()),
                                                                     LAYER_OBJECTS+1));
    }
  }
}

HitResponse
WaterDrop::collision(GameObject& other, const CollisionHit& )
{
  return FORCE_MOVE;
}

/* EOF */
