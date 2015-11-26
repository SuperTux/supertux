//  SuperTux - BicyclePlatform
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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

#include "object/bicycle_platform.hpp"

#include <algorithm>
#include <math.h>

#include "object/player.hpp"
#include "object/portable.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

BicyclePlatform::BicyclePlatform(const ReaderMapping& reader) :
  MovingSprite(reader, LAYER_OBJECTS, COLGROUP_STATIC),
  master(0),
  slave(0),
  center(),
  radius(128),
  angle(0),
  angular_speed(0),
  contacts(),
  momentum(0)
{
  center = get_pos();
}

BicyclePlatform::BicyclePlatform(BicyclePlatform* master_) :
  MovingSprite(*master_),
  master(master_),
  slave(this),
  center(master->center),
  radius(master->radius),
  angle(master->angle + M_PI),
  angular_speed(0),
  contacts(),
  momentum(0)
{
  set_pos(get_pos() + Vector(master->get_bbox().get_width(), 0));
  master->master = master;
  master->slave = this;
}

BicyclePlatform::~BicyclePlatform()
{
  if ((this == master) && (master)) {
    slave->master = 0;
    slave->slave = 0;
  }
  if ((master) && (this == slave)) {
    master->master = 0;
    master->slave = 0;
  }
  master = 0;
  slave = 0;
}

HitResponse
BicyclePlatform::collision(GameObject& other, const CollisionHit& )
{

  // somehow the hit parameter does not get filled in, so to determine (hit.top == true) we do this:
  MovingObject* mo = dynamic_cast<MovingObject*>(&other);
  if (!mo) return FORCE_MOVE;
  if ((mo->get_bbox().p2.y) > (bbox.p1.y + 2)) return FORCE_MOVE;

  Player* pl = dynamic_cast<Player*>(mo);
  if (pl) {
    if (pl->is_big()) momentum += 0.1 * Sector::current()->get_gravity();
    Portable* po = pl->get_grabbed_object();
    MovingObject* pomo = dynamic_cast<MovingObject*>(po);
    if (contacts.insert(pomo).second) momentum += 0.1 * Sector::current()->get_gravity();
  }

  if (contacts.insert(&other).second) momentum += 0.1 * Sector::current()->get_gravity();
  return FORCE_MOVE;
}

void
BicyclePlatform::update(float elapsed_time)
{
  if (!slave) {
    Sector::current()->add_object(std::make_shared<BicyclePlatform>(this));
    return;
  }
  if (!master) {
    return;
  }
  if (this == slave) {
    angle = master->angle + M_PI;
    while (angle < 0) { angle += 2*M_PI; }
    while (angle > 2*M_PI) { angle -= 2*M_PI; }
    Vector dest_ = center + Vector(cosf(angle), sinf(angle)) * radius - (bbox.get_size().as_vector() * 0.5);
    movement = dest_ - get_pos();
  }
  if (this == master) {
    float momentum_diff = momentum - slave->momentum;
    contacts.clear(); momentum = 0;
    slave->contacts.clear(); slave->momentum = 0;

    float angular_momentum = cosf(angle) * momentum_diff;

    angular_speed += (angular_momentum * elapsed_time) * M_PI;
    angular_speed *= 1 - elapsed_time * 0.2;
    angle += angular_speed * elapsed_time;
    while (angle < 0) { angle += 2*M_PI; }
    while (angle > 2*M_PI) { angle -= 2*M_PI; }
    angular_speed = std::min(std::max(angular_speed, static_cast<float>(-128*M_PI*elapsed_time)), static_cast<float>(128*M_PI*elapsed_time));
    Vector dest_ = center + Vector(cosf(angle), sinf(angle)) * radius - (bbox.get_size().as_vector() * 0.5);
    movement = dest_ - get_pos();

    center += Vector(angular_speed, 0) * elapsed_time * 32;
    slave->center += Vector(angular_speed, 0) * elapsed_time * 32;

  }
}

/* EOF */
