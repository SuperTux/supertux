//  SuperTux
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#include "badguy/skydive.hpp"

#include "object/explosion.hpp"
#include "object/player.hpp"
#include "supertux/constants.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"

SkyDive::SkyDive(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/skydive/skydive.sprite"),
  is_grabbed(false)
{
}

void
SkyDive::collision_solid(const CollisionHit& hit)
{
  if (hit.bottom) {
    explode ();
    return;
  }

  if (hit.left || hit.right)
    physic.set_velocity_x (0.0);
} /* void collision_solid */

HitResponse
SkyDive::collision_badguy(BadGuy&, const CollisionHit& hit)
{
  if (hit.bottom) {
    explode();
    return ABORT_MOVE;
  }

  return FORCE_MOVE;
} /* HitResponse collision_badguy */

void
SkyDive::grab(MovingObject&, const Vector& pos, Direction dir_)
{
  movement = pos - get_pos();
  dir = dir_;

  is_grabbed = true;

  physic.set_velocity_x(movement.x * LOGICAL_FPS);
  physic.set_velocity_y(0.0);
  physic.set_acceleration_y(0.0);
  physic.enable_gravity(false);
  set_colgroup_active(COLGROUP_DISABLED);
}

void
SkyDive::ungrab(MovingObject& , Direction)
{
  is_grabbed = false;

  physic.set_velocity_y(0);
  physic.set_acceleration_y(0);
  physic.enable_gravity(true);
  set_colgroup_active(COLGROUP_MOVING);
}

HitResponse
SkyDive::collision_player(Player&, const CollisionHit& hit)
{
  if (hit.bottom) {
    explode();
    return ABORT_MOVE;
  }

  return FORCE_MOVE;
} /* HitResponse collision_player */

bool
SkyDive::collision_squished(GameObject& obj)
{
  auto player = dynamic_cast<Player *>(&obj);
  if (player) {
    player->bounce(*this);
    return false;
  }

  explode();
  return false;
} /* bool collision_squished */

void
SkyDive::collision_tile(uint32_t tile_attributes)
{
  if(tile_attributes & Tile::HURTS)
  {
    explode();
  }
}
void
SkyDive::active_update(float elapsed_time)
{
  if (!is_grabbed)
    movement = physic.get_movement(elapsed_time);
} /* void active_update */

void
SkyDive::explode()
{
  if (!is_valid())
    return;

  auto explosion = std::make_shared<Explosion>(get_anchor_pos (bbox, ANCHOR_BOTTOM));

  explosion->hurts(true);
  explosion->pushes(false);
  Sector::current()->add_object(explosion);

  remove_me();
} /* void explode */

/* vim: set sw=2 sts=2 et fdm=marker : */
/* EOF */
