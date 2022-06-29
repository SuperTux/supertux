//  SkullyHop - A Hopping Skull
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_BADGUY_SKULLYHOP_HPP
#define HEADER_SUPERTUX_BADGUY_SKULLYHOP_HPP

#include "badguy/badguy.hpp"

/** Badguy "SkullyHop" - A Hopping Skull */
class SkullyHop final : public BadGuy
{
public:
  SkullyHop(const ReaderMapping& reader);

  virtual void initialize() override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit) override;
  virtual bool collision_squished(GameObject& object) override;
  virtual void active_update(float dt_sec) override;

  virtual void unfreeze(bool melt = true) override;
  virtual bool is_freezable() const override;
  virtual std::string get_class() const override { return "skullyhop"; }
  virtual std::string get_display_name() const override { return _("Skullyhop"); }

private:
  enum SkullyHopState {
    STANDING,
    CHARGING,
    JUMPING
  };

private:
  void set_state(SkullyHopState newState);

private:
  Timer recover_timer;
  SkullyHopState state;

private:
  SkullyHop(const SkullyHop&) = delete;
  SkullyHop& operator=(const SkullyHop&) = delete;
};

#endif

/* EOF */
