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

#include "badguy/granito_big.hpp"

GranitoBig::GranitoBig(const ReaderMapping& reader) :
  Granito(reader, "images/creatures/granito/big/granito_big.sprite", LAYER_OBJECTS - 2),
  m_carrying(nullptr)
{
  parse_type(reader);

  max_drop_height = 16;
}

HitResponse
GranitoBig::collision_player(Player& player, const CollisionHit& hit)
{
  // Prevent from triggering STATE_LOOKUP
  return FORCE_MOVE;
}

void
GranitoBig::active_update(float dt_sec)
{
  Granito::active_update(dt_sec);

  if (!m_carrying) return;

  Vector pos(get_bbox().get_middle().x - m_carrying->get_bbox().get_width() / 2,
             get_bbox().get_top() - m_carrying->get_bbox().get_height());
  m_carrying->set_pos(pos);
  m_carrying->set_action("sit", m_dir);
}

GameObjectTypes
GranitoBig::get_types() const
{
  return {
    { "default", _("Default") },
    { "standing", _("Standing") },
    { "walking", _("Walking") }
  };
}

bool
GranitoBig::try_wave()
{
  return false;
}

bool
GranitoBig::try_jump()
{
  return false;
}

/* EOF */
