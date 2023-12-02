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
{
  walk_speed = IGEL_SPEED;
  max_drop_height = 16;
}

void
Igel::active_update(float dt_sec)
{
  WalkingBadguy::active_update(dt_sec);
}

bool
Igel::is_freezable() const
{
  return true;
}

/**bool
Igel::collision_squished(GameObject& )
{
  // This will hurt.
  return false;
}*/
// Enable this and the igle will no longer be butt-jumpable.
// Don't forget to enable it in .hpp too!

/* EOF */
