//  SuperTux
//  Copyright (C) 2026 Francisco Gama Franco Soares Martins
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

#include "st_assert.hpp"
#include "supertux/physic.hpp"

#include <algorithm>

// Provide the Physic constructor so we don't need to compile
// physic.cpp (which drags in Sector and half the engine).
Physic::Physic() :
  ax(0), ay(0),
  vx(0), vy(0),
  gravity_enabled_flag(true),
  gravity_modifier(1.0f)
{
}

// Water-physics constants mirrored from HeavyCoin::update().
static const float WATER_GRAVITY_MODIFIER = 0.1f;
static const float WATER_MAX_DROP_SPEED = 100.f;

// Reproduces the water-physics logic from HeavyCoin::update()
// without needing a Sector or tile lookup.
void apply_water_physics(Physic& physic, bool in_water)
{
  physic.set_gravity_modifier(in_water ? WATER_GRAVITY_MODIFIER : 1.f);
  if (in_water)
    physic.set_velocity_y(std::min(physic.get_velocity_y(), WATER_MAX_DROP_SPEED));
}

int main(void)
{
  // Gravity should be reduced to 10% when a coin is inside water.(We use this value to ensure the coin sinks slowly)
  Physic p1;
  apply_water_physics(p1, true);
  ST_ASSERT("in water: gravity modifier is reduced",
            p1.get_gravity_modifier() == WATER_GRAVITY_MODIFIER);

  // Gravity should stay at 100% when a coin is outside water.
  Physic p2;
  apply_water_physics(p2, false);
  ST_ASSERT("out of water: gravity modifier is normal",
            p2.get_gravity_modifier() == 1.0f);

  // A coin falling fast (500 px/s) should be capped to 100 px/s on water entry.
  Physic p3;
  p3.set_velocity_y(500.f);
  apply_water_physics(p3, true);
  ST_ASSERT("in water: fast fall velocity is capped",
            p3.get_velocity_y() <= WATER_MAX_DROP_SPEED);

  // A coin already falling slowly (50 px/s) should keep its speed in water.
  Physic p4;
  p4.set_velocity_y(50.f);
  apply_water_physics(p4, true);
  ST_ASSERT("in water: slow fall velocity unchanged",
            p4.get_velocity_y() == 50.f);

  // Upward velocity (negative) must not be clamped by the water cap.
  Physic p5;
  p5.set_velocity_y(-200.f);
  apply_water_physics(p5, true);
  ST_ASSERT("in water: upward velocity not capped",
            p5.get_velocity_y() == -200.f);

  // Outside water, velocity should never be capped regardless of speed.
  Physic p6;
  p6.set_velocity_y(500.f);
  apply_water_physics(p6, false);
  ST_ASSERT("out of water: velocity not capped",
            p6.get_velocity_y() == 500.f);

  // Entering water caps velocity; leaving water restores full gravity.
  Physic p7;
  p7.set_velocity_y(400.f);
  apply_water_physics(p7, true);
  ST_ASSERT("transition: velocity capped on entry",
            p7.get_velocity_y() == WATER_MAX_DROP_SPEED);
  apply_water_physics(p7, false);
  ST_ASSERT("transition: gravity restored on exit",
            p7.get_gravity_modifier() == 1.0f);

  return 0;
}

/* EOF */
