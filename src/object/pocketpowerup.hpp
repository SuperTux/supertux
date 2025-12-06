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

#pragma once

#include "object/powerup.hpp"

/// PowerUp that flings itself upwards
/// can't be collected right away.
class PocketPowerUp : public PowerUp
{
public:
  PocketPowerUp(PlayerBonusType bonustype, Vector pos);
  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

public:
  Timer m_cooldown_timer;
  Timer m_blink_timer;
  bool m_visible;

private:
  PocketPowerUp(const PocketPowerUp&) = delete;
  PocketPowerUp& operator=(const PocketPowerUp&) = delete;
};

