//  SuperTux
//  Copyright (C) 2023 MatusGuy
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

#include "badguy/granito_giant.hpp"

#include "sprite/sprite.hpp"

GranitoGiant::GranitoGiant(const ReaderMapping& reader)
  : MovingSprite(reader, "images/creatures/granito/giant/granito_giant.sprite")
{
  parse_type(reader);

  m_col.m_group = COLGROUP_MOVING_STATIC;
  m_col.set_unisolid(true);
}

HitResponse GranitoGiant::collision(GameObject &other, const CollisionHit &hit)
{
  return FORCE_MOVE;
}

GameObjectTypes
GranitoGiant::get_types() const
{
  return {
    { "stand", _("Standing") },
    { "sleep", _("Sleeping") }
  };
}

/* EOF */
