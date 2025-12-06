//  SuperTux
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
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

#include "object/pocketpowerup.hpp"
#include "supertux/player_status.hpp"

PocketPowerUp::PocketPowerUp(PlayerBonusType bonustype, Vector pos):
  PowerUp(pos, PowerUp::get_type_from_bonustype(bonustype)),
  m_cooldown_timer(),
  m_blink_timer(),
  m_visible(true)
{
  physic.set_velocity_y(-325.f);
  physic.set_gravity_modifier(0.4f);
  set_layer(LAYER_FOREGROUND1);
  m_col.m_group = COLGROUP_DISABLED;
}

void
PocketPowerUp::PocketPowerUp::update(float dt_sec)
{
  PowerUp::update(dt_sec);

  bool check = m_cooldown_timer.check();
  if (!m_cooldown_timer.started() && !check && m_col.m_group != COLGROUP_TOUCHABLE)
  {
    m_cooldown_timer.start(1.3f);
    m_blink_timer.start(.15f, true);
  }

  if (check)
  {
    m_visible = true;
    m_blink_timer.stop();
    m_col.m_group = COLGROUP_TOUCHABLE;
  }

  if (m_blink_timer.check())
    m_visible = !m_visible;
}
void

PocketPowerUp::PocketPowerUp::draw(DrawingContext& context)
{
  if (!m_visible)
    return;

  PowerUp::draw(context);
}

