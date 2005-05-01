//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include <config.h>

#include <math.h>
#include "bullet.h"
#include "resources.h"
#include "camera.h"
#include "sector.h"
#include "app/globals.h"
#include "sprite/sprite_manager.h"
#include "badguy/badguy.h"

static const float BULLET_XM = 300;
static const float BULLET_STARTING_YM = 0;

Bullet::Bullet(const Vector& pos, float xm, int dir, int kind_)
  : kind(kind_), life_count(3), sprite(0)
{
  bbox.set_pos(pos);
  bbox.set_size(4, 4);

  float speed = dir == RIGHT ? BULLET_XM : -BULLET_XM;
  physic.set_velocity_x(speed + xm);
  physic.set_velocity_y(-BULLET_STARTING_YM);

  if (kind == ICE_BULLET) {
    life_count = 6; //ice-bullets get "extra lives" for bumping off walls
    sprite = sprite_manager->create("icebullet");
  } else if(kind == FIRE_BULLET) {
    sprite = sprite_manager->create("firebullet");
  }
}

Bullet::~Bullet()
{
  delete sprite;
}

void
Bullet::action(float elapsed_time)
{
  if(kind == FIRE_BULLET) {
    // @not completely framerate independant :-/
    physic.set_velocity_y(physic.get_velocity_y() - 50 * elapsed_time);
  }
  if(physic.get_velocity_y() > 900)
    physic.set_velocity_y(900);
  else if(physic.get_velocity_y() < -900)
    physic.set_velocity_y(-900);

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

HitResponse
Bullet::collision(GameObject& other, const CollisionHit& hit)
{
  if(other.get_flags() & FLAG_SOLID) {
    if(fabsf(hit.normal.y) > .5) { // roof or floor bump
      physic.set_velocity_y(-physic.get_velocity_y());
      life_count -= 1;
    } else { // bumped left or right
      if(kind == FIRE_BULLET)
        remove_me();
      else
        physic.set_velocity_x(-physic.get_velocity_x());
    }
    
    return CONTINUE;
  }

  BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
  if(badguy) {
    badguy->kill_fall();
    remove_me();
    return FORCE_MOVE;
  }
 
  return FORCE_MOVE;
}


