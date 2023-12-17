//  SuperTux - Corrupted Root
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

#ifndef HEADER_SUPERTUX_BADGUY_ROOT_HPP
#define HEADER_SUPERTUX_BADGUY_ROOT_HPP

#include "badguy/badguy.hpp"
#include "supertux/timer.hpp"

//TODO: Roots for all directions
class Root final : public BadGuy
{
public:
  Root(const Vector& pos, const std::string& sprite);

  virtual void initialize() override;
  virtual void draw(DrawingContext &context) override;
  virtual void active_update(float dt_sec) override;

  virtual HitResponse collision_badguy(BadGuy &other, const CollisionHit &hit) override;

  virtual bool is_flammable() const override { return false; }
  virtual bool is_freezable() const override { return false; }
  virtual bool is_snipable () const override { return false; }

private:
  enum State { STATE_HATCHING, STATE_APPEARING, STATE_RETREATING };

  SurfacePtr m_base_surface;
  Timer m_timer;
  State m_state;
  float m_offset;
  float m_maxheight;
};

#endif

/* EOF */
