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

#include "object/player.hpp"
#include "object/star.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"

static const float INITIALJUMP = -400;
static const float STAR_SPEED = 150;
static const float JUMPSTAR_SPEED = -300;

Star::Star(const Vector& pos, Direction direction) :
  MovingSprite(pos, "images/powerups/star/star.sprite", LAYER_OBJECTS, COLGROUP_MOVING),
  physic(),
  light(0.0f,0.0f,0.0f),
  lightsprite(sprite_manager->create("images/objects/lightmap_light/lightmap_light-tiny.sprite"))
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
}

void
Star::draw(DrawingContext& context){
  //Draw the Sprite.
  MovingSprite::draw(context);
  //Draw the light when dark
  context.get_light( get_bbox().get_middle(), &light );
  if (light.red + light.green + light.blue < 3.0){
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
