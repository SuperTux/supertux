//  SuperTux
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

#ifndef HEADER_SUPERTUX_BADGUY_JUMPY_HPP
#define HEADER_SUPERTUX_BADGUY_JUMPY_HPP

#include "badguy/badguy.hpp"

class Jumpy final : public BadGuy
{
public:
  Jumpy(const ReaderMapping& reader);

  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision_badguy(BadGuy& other, const CollisionHit& hit) override;

  virtual void active_update(float) override;
  virtual void on_flip(float height) override;

  virtual void freeze() override;
  virtual bool is_freezable() const override;
  virtual bool is_flammable() const override;
  virtual std::string get_class() const override { return "jumpy"; }
  virtual std::string get_display_name() const override { return _("Jumpy"); }

private:
  HitResponse hit(const CollisionHit& hit);

private:
  Vector pos_groundhit;
  bool groundhit_pos_set;

private:
  Jumpy(const Jumpy&) = delete;
  Jumpy& operator=(const Jumpy&) = delete;
};

#endif

/* EOF */
