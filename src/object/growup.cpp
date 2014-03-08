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

#include <math.h>

#include "audio/sound_manager.hpp"
#include "object/growup.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"

GrowUp::GrowUp(Direction direction) :
  MovingSprite(Vector(0,0), "images/powerups/egg/egg.sprite", LAYER_OBJECTS, COLGROUP_MOVING),
  physic(),
  light(0.0f,0.0f,0.0f),
  lightsprite(sprite_manager->create("images/objects/lightmap_light/lightmap_light-small.sprite"))
{
  physic.enable_gravity(true);
  physic.set_velocity_x((direction == LEFT)?-100:100);
  sound_manager->preload("sounds/grow.ogg");
  //set light for glow effect
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(0.2f, 0.2f, 0.0f));
}

void
GrowUp::update(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time);
}

void
GrowUp::draw(DrawingContext& context){
  //Set Sprite rotation angle
  sprite->set_angle(get_pos().x * 360.0f / (32.0f * M_PI));
  //Draw the Sprite.
  MovingSprite::draw(context);
  //Draw the light when dark
  context.get_light( get_bbox().get_middle(), &light );
  if (light.red + light.green < 2.0){
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    lightsprite->draw(context, get_bbox().get_middle(), 0);
    context.pop_target();
  }
}

void
GrowUp::collision_solid(const CollisionHit& hit)
{
  if(hit.top)
    physic.set_velocity_y(0);
  if(hit.bottom && physic.get_velocity_y() > 0)
    physic.set_velocity_y(0);
  if(hit.left || hit.right)
    physic.set_velocity_x(-physic.get_velocity_x());
}

HitResponse
GrowUp::collision(GameObject& other, const CollisionHit& hit )
{
  Player* player = dynamic_cast<Player*>(&other);
  if(player != 0) {
    if(!player->add_bonus(GROWUP_BONUS, true)){
      // Tux can't grow right now.
      collision_solid( hit );
      return ABORT_MOVE;
    }

    sound_manager->play("sounds/grow.ogg");
    remove_me();

    return ABORT_MOVE;
  }

  return FORCE_MOVE;
}

void
GrowUp::do_jump()
{
  physic.set_velocity_y(-300);
}

/* EOF */
