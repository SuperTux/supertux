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

#ifndef HEADER_SUPERTUX_BADGUY_GRANITO_BIG_HPP
#define HEADER_SUPERTUX_BADGUY_GRANITO_BIG_HPP

#include "badguy/granito.hpp"

class GranitoBig final : public Granito
{
public:
  GranitoBig(const ReaderMapping& reader);

  virtual HitResponse collision_player(Player& player, const CollisionHit& hit) override;

  virtual void active_update(float dt_sec) override;

  static std::string class_name() { return "granito_big"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Big Granito"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual GameObjectTypes get_types() const override;

  Granito* m_carrying;

protected:
  bool try_wave() override;
  bool try_jump() override;

private:
  GranitoBig(const GranitoBig&) = delete;
  GranitoBig& operator=(const GranitoBig&) = delete;
};

#endif

/* EOF */
