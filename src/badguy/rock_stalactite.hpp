//  SuperTux
//  Copyright (C) 2022 Vankata453
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

#ifndef HEADER_SUPERTUX_BADGUY_ROCK_STALACTITE_HPP
#define HEADER_SUPERTUX_BADGUY_ROCK_STALACTITE_HPP

#include "badguy/stalactite.hpp"

class RockStalactite final : public Stalactite
{
public:
  RockStalactite(const ReaderMapping& mapping);

  virtual HitResponse collision_bullet(Bullet& bullet, const CollisionHit& hit) override;

  static std::string class_name() { return "rock_stalactite"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Rock Stalactite"); }
  virtual std::string get_display_name() const override { return display_name(); }

private:
  RockStalactite(const RockStalactite&) = delete;
  RockStalactite& operator=(const RockStalactite&) = delete;
};

#endif

/* EOF */
