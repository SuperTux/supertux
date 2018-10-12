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

  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit) override;
  virtual void active_update(float elapsed_time) override;

  virtual void freeze() override;
  virtual bool is_freezable() const override;
  virtual bool is_flammable() const override;

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

class LiveFireAsleep final : public LiveFire
{
public:
  LiveFireAsleep(const ReaderMapping& reader);

  virtual void initialize() override;
  virtual std::string get_class() const override {
    return "livefire_asleep";
  }
  virtual std::string get_display_name() const override {
    return _("Sleeping live fire");
  }
};

class LiveFireDormant final : public LiveFire
{
public:
  LiveFireDormant(const ReaderMapping& reader);

  virtual void initialize() override;
  virtual std::string get_class() const override {
    return "livefire_dormant";
  }
  virtual std::string get_display_name() const override {
    return _("Dormant live fire");
  }
};

#endif

/* EOF */
