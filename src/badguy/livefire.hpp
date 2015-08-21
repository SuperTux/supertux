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

  void collision_solid(const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
  void active_update(float elapsed_time);

  void freeze();
  bool is_freezable() const;
  bool is_flammable() const;

  virtual void kill_fall();
  virtual std::string get_class() {
    return "livefire";
  }

  virtual ObjectSettings get_settings();

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
  virtual ObjectSettings get_settings();
};

class LiveFireDormant : public LiveFire
{
public:
  LiveFireDormant(const ReaderMapping& reader);

  void initialize();
  virtual ObjectSettings get_settings();
};

#endif

/* EOF */
