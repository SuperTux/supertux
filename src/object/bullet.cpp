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

#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/main.hpp"
#include "supertux/sector.hpp"

namespace {
  const float BULLET_XM = 600;
  const float BULLET_STARTING_YM = 0;
}

Bullet::Bullet(const Vector& pos, float xm, int dir, BonusType type)
  : life_count(3), type(type)
{
  float speed = dir == RIGHT ? BULLET_XM : -BULLET_XM;
  physic.set_velocity_x(speed + xm);

  if(type == FIRE_BONUS) {
    sprite.reset(sprite_manager->create("images/objects/bullets/firebullet.sprite"));
  } else if(type == ICE_BONUS) {
    life_count = 10;
    sprite.reset(sprite_manager->create("images/objects/bullets/icebullet.sprite"));
  } else {
    log_warning << "Bullet::Bullet called with unknown BonusType" << std::endl;
    life_count = 10;
    sprite.reset(sprite_manager->create("images/objects/bullets/firebullet.sprite"));
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
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
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
