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

#ifndef HEADER_SUPERTUX_BADGUY_GRANITO_GIANT_HPP
#define HEADER_SUPERTUX_BADGUY_GRANITO_GIANT_HPP

#include "object/moving_sprite.hpp"

class GranitoGiant final : public MovingSprite
{
public:
  explicit GranitoGiant(const ReaderMapping& reader);

  virtual HitResponse collision(GameObject &other, const CollisionHit &hit) override;

  static std::string class_name() { return "granito_giant"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Giant Granito"); }
  virtual std::string get_display_name() const override { return display_name(); }

  GameObjectTypes get_types() const override;

private:
  enum Type { STAND, SLEEP };

private:
  GranitoGiant(const GranitoGiant&) = delete;
  GranitoGiant& operator=(const GranitoGiant&) = delete;
};

#endif

/* EOF */
