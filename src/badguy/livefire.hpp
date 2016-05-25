//  SuperTux badguy - walking flame that glows
//  Copyright (C) 2013 LMH <lmh.0013@gmail.com>
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

#ifndef HEADER_SUPERTUX_BADGUY_LiveFire_HPP
#define HEADER_SUPERTUX_BADGUY_LiveFire_HPP

#include "badguy/walking_badguy.hpp"

class LiveFire : public WalkingBadguy
{
public:
  LiveFire(const ReaderMapping& reader);

  void collision_solid(const CollisionHit& hit) override;
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit) override;
  void active_update(float elapsed_time) override;

  void freeze() override;
  bool is_freezable() const override;
  bool is_flammable() const override;

  virtual void kill_fall() override;
  virtual std::string get_class() const override {
    return "livefire";
  }
  virtual std::string get_display_name() const override {
    return _("Live fire");
  }

private:
  std::string death_sound;

protected:
  enum SState {
    STATE_SLEEPING,
    STATE_WAKING,
    STATE_WALKING,
    STATE_DORMANT
  };
  SState state;
};

class LiveFireAsleep : public LiveFire
{
public:
  LiveFireAsleep(const ReaderMapping& reader);

  void initialize();
  std::string get_display_name() const {
    return _("Sleeping live fire");
  }
};

class LiveFireDormant : public LiveFire
{
public:
  LiveFireDormant(const ReaderMapping& reader);

  void initialize();
  std::string get_display_name() const {
    return _("Dormant live fire");
  }
};

#endif

/* EOF */
