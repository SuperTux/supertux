//  SuperTux - Badguy "Igel"
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "badguy/igel.hpp"

#include "object/bullet.hpp"
#include "supertux/sector.hpp"

namespace {

const float IGEL_SPEED = 80; /**< Speed at which we walk around. */
const float TURN_RECOVER_TIME = 0.5; /**< Seconds before we will again turn around when shot at. */
const float RANGE_OF_VISION = 256; /**< Sange in px at which we can see bullets. */

} // namespace

Igel::Igel(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/igel/igel.sprite", "left", "right"),
  turn_recover_timer()
{
  walk_speed = IGEL_SPEED;
  max_drop_height = 16;
}

void
Igel::be_normal()
{
  initialize();
}

void
Igel::turn_around()
{
  WalkingBadguy::turn_around();
  turn_recover_timer.start(TURN_RECOVER_TIME);
}

bool
Igel::can_see(const MovingObject& o) const
{
  Rectf ob = o.get_bbox();

  bool inReach_left = ((ob.get_right() < m_col.m_bbox.get_left()) && (ob.get_right() >= m_col.m_bbox.get_left()-((m_dir == Direction::LEFT) ? RANGE_OF_VISION : 0)));
  bool inReach_right = ((ob.get_left() > m_col.m_bbox.get_right()) && (ob.get_left() <= m_col.m_bbox.get_right()+((m_dir == Direction::RIGHT) ? RANGE_OF_VISION : 0)));
  bool inReach_top = (ob.get_bottom() >= m_col.m_bbox.get_top());
  bool inReach_bottom = (ob.get_top() <= m_col.m_bbox.get_bottom());

  return ((inReach_left || inReach_right) && inReach_top && inReach_bottom);
}

void
Igel::active_update(float dt_sec)
{
  bool wants_to_flee = false;

  // Check if we see a fire bullet.
  for (const auto& bullet : Sector::get().get_objects_by_type<Bullet>()) {
    if (bullet.get_type() != FIRE_BONUS) continue;
    if (can_see(bullet)) wants_to_flee = true;
  }

  // If the Igel wants to flee and the turn recovery timer is not started,
  // turn around and handle the fleeing behavior.
  if (wants_to_flee && (!turn_recover_timer.started())) {
    turn_around();
    BadGuy::active_update(dt_sec);
    return;
  }

  // Otherwise, adhere to the default behavior for WalkingBadguy.
  WalkingBadguy::active_update(dt_sec);
}

HitResponse
Igel::collision_bullet(Bullet& bullet, const CollisionHit& hit)
{
  // Default reaction if hit on the front side or for freeze and unfreeze conditions.
  if (((m_dir == Direction::LEFT) && hit.left) || ((m_dir == Direction::RIGHT) && hit.right) ||
    (bullet.get_type() == ICE_BONUS) || ((bullet.get_type() == FIRE_BONUS) && (m_frozen))) {
    return BadGuy::collision_bullet(bullet, hit);
  }

  // Otherwise, make the bullet ricochet and ignore the hit for this case.
  bullet.ricochet(*this, hit);
  return FORCE_MOVE;
}

bool
Igel::is_freezable() const
{
  return true;
}

/**bool
Igel::collision_squished(GameObject& )
{
  // this will hurt
  return false;
}*/
// Enable this and the igle will no longer be butt-jumpable.
// Don't forget to enable it in .hpp too!

/* EOF */
