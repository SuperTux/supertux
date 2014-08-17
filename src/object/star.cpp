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

#include "math/random_generator.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "object/star.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

static const float INITIALJUMP = -400;
static const float STAR_SPEED = 150;
static const float JUMPSTAR_SPEED = -300;

Star::Star(const Vector& pos, Direction direction) :
  MovingSprite(pos, "images/powerups/star/star.sprite", LAYER_OBJECTS, COLGROUP_MOVING),
  physic(),
  light(0.0f,0.0f,0.0f),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-small.sprite"))
{
  physic.set_velocity((direction == LEFT) ? -STAR_SPEED : STAR_SPEED, INITIALJUMP);
  //set light for glow effect
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(0.4f, 0.4f, 0.4f));
}

void
Star::update(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time);

  // when near Tux, spawn particles
  Player* player = Sector::current()->get_nearest_player (this->get_bbox ());
  if (player) {
    float disp_x = player->get_bbox().p1.x - bbox.p1.x;
    float disp_y = player->get_bbox().p1.y - bbox.p1.y;
    if (disp_x*disp_x + disp_y*disp_y <= 256*256)
    {
      if (graphicsRandom.rand(0, 2) == 0) {
        float px = graphicsRandom.randf(bbox.p1.x+0, bbox.p2.x-0);
        float py = graphicsRandom.randf(bbox.p1.y+0, bbox.p2.y-0);
        Vector ppos = Vector(px, py);
        Vector pspeed = Vector(0, 0);
        Vector paccel = Vector(0, 0);
        Sector::current()->add_object(new SpriteParticle("images/objects/particles/sparkle.sprite",
                                                         // draw bright sparkles when very close to Tux, dark sparkles when slightly further
                                                         (disp_x*disp_x + disp_y*disp_y <= 128*128) ?
                                                         // make every other a longer sparkle to make trail a bit fuzzy
                                                         (size_t(game_time*20)%2) ? "small" : "medium" : "dark", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS+1+5));
      }
    }
  }
}

void
Star::draw(DrawingContext& context){
  //Draw the Sprite.
  MovingSprite::draw(context);
  //Draw the light when dark
  context.get_light( get_bbox().get_middle(), &light );
  if (light.red + light.green + light.blue < 3.0){
    MovingSprite::draw(context);
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    lightsprite->draw(context, get_bbox().get_middle(), 0);
    context.pop_target();
  }
}

void
Star::collision_solid(const CollisionHit& hit)
{
  if(hit.bottom) {
    physic.set_velocity_y(JUMPSTAR_SPEED);
  } else if(hit.top) {
    physic.set_velocity_y(0);
  } else if(hit.left || hit.right) {
    physic.set_velocity_x(-physic.get_velocity_x());
  }
}

HitResponse
Star::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    player->make_invincible();
    remove_me();
    return ABORT_MOVE;
  }

  return FORCE_MOVE;
}

/* EOF */
