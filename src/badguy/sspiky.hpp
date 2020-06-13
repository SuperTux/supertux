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

#ifndef HEADER_SUPERTUX_BADGUY_SSPIKY_HPP
#define HEADER_SUPERTUX_BADGUY_SSPIKY_HPP

#include "badguy/walking_badguy.hpp"

class SSpiky final : public WalkingBadguy
{
public:
  SSpiky(const ReaderMapping& reader);

  virtual void initialize() override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit) override;
  virtual void active_update(float dt_sec) override;

  virtual void freeze() override;
  virtual bool is_freezable() const override;
  virtual bool is_flammable() const override;
  virtual std::string get_class() const override { return "sspiky"; }
  virtual std::string get_display_name() const override { return _("Sleeping Spiky"); }

protected:
  enum SSpikyState {
    SSPIKY_SLEEPING,
    SSPIKY_WAKING,
    SSPIKY_WALKING
  };
  SSpikyState state;

private:
  SSpiky(const SSpiky&) = delete;
  SSpiky& operator=(const SSpiky&) = delete;
};

#endif

/* EOF */
