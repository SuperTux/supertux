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
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/sector.hpp"

namespace {
const float BULLET_XM = 600;
const float BULLET_STARTING_YM = 0;
}

Bullet::Bullet(const Vector& pos, float xm, int dir, BonusType type) :
  physic(),
  life_count(3), 
  sprite(),
  light(0.0f,0.0f,0.0f),
  lightsprite(sprite_manager->create("images/objects/lightmap_light/lightmap_light-small.sprite")),
  type(type)
{
  float speed = dir == RIGHT ? BULLET_XM : -BULLET_XM;
  physic.set_velocity_x(speed + xm);

  if(type == FIRE_BONUS) {
    sprite = sprite_manager->create("images/objects/bullets/firebullet.sprite");
    lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
    lightsprite->set_color(Color(0.3f, 0.1f, 0.0f));
 } else if(type == ICE_BONUS) {
    life_count = 10;
    sprite = sprite_manager->create("images/objects/bullets/icebullet.sprite");
  } else {
    log_warning << "Bullet::Bullet called with unknown BonusType" << std::endl;
    life_count = 10;
    sprite = sprite_manager->create("images/objects/bullets/firebullet.sprite");
  }

  bbox.set_pos(pos);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
}

Bullet::~Bullet()
{
}

void
Bullet::update(float elapsed_time)
{
  // cause fireball color to flicker randomly
  if (gameRandom.rand(5) != 0) {
    lightsprite->set_color(Color(0.3f + gameRandom.rand(10)/100.0f, 0.1f + gameRandom.rand(20)/100.0f, gameRandom.rand(10)/100.0f));
  } else
    lightsprite->set_color(Color(0.3f, 0.1f, 0.0f));
  // remove bullet when it's offscreen
  float scroll_x =
    Sector::current()->camera->get_translation().x;
  float scroll_y =
    Sector::current()->camera->get_translation().y;
  if (get_pos().x < scroll_x ||
      get_pos().x > scroll_x + SCREEN_WIDTH ||
      //     get_pos().y < scroll_y ||
      get_pos().y > scroll_y + SCREEN_HEIGHT ||
      life_count <= 0) {
    remove_me();
    return;
  }

  movement = physic.get_movement(elapsed_time);
}

void
Bullet::draw(DrawingContext& context)
{
  //Draw the Sprite.
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
  //Draw the light if fire and dark
  if(type == FIRE_BONUS){
    context.get_light( get_bbox().get_middle(), &light );
    if (light.red + light.green < 2.0){
      context.push_target();
      context.set_target(DrawingContext::LIGHTMAP);
      sprite->draw(context, get_pos(), LAYER_OBJECTS);
      lightsprite->draw(context, get_bbox().get_middle(), 0);
      context.pop_target();
    }
  }
}

void
Bullet::collision_solid(const CollisionHit& hit)
{
  if(hit.top || hit.bottom) {
    physic.set_velocity_y(-physic.get_velocity_y());
    life_count--;
  } else if(hit.left || hit.right) {
    if(type == ICE_BONUS) {
      physic.set_velocity_x(-physic.get_velocity_x());
      life_count--;
    } else
      remove_me();
  }
}

void
Bullet::ricochet(GameObject& , const CollisionHit& hit)
{
  collision_solid(hit);
}

HitResponse
Bullet::collision(GameObject& , const CollisionHit& )
{
  return FORCE_MOVE;
}

/* EOF */
