//  SuperTux BadGuy GoldBomb - a bomb that throws up coins when exploding
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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


#ifndef HEADER_SUPERTUX_BADGUY_GOLDBOMB_HPP
#define HEADER_SUPERTUX_BADGUY_GOLDBOMB_HPP

#include "audio/sound_source.hpp"
#include "badguy/walking_badguy.hpp"
#include "object/portable.hpp"

class GoldBomb : public WalkingBadguy, public Portable
{
public:
  GoldBomb(const Reader& reader);

  void collision_solid(const CollisionHit& hit);
  HitResponse collision(GameObject& object, const CollisionHit& hit);
  HitResponse collision_player(Player& player, const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);

  void active_update(float elapsed_time);

  void grab(MovingObject& object, const Vector& pos, Direction dir);
  void ungrab(MovingObject& object, Direction dir);
  bool is_portable() const;

  void freeze();
  bool is_freezable() const;

  void kill_fall();
  virtual std::string get_class() const {
    return "goldbomb";
  }

  virtual ObjectSettings get_settings();
  void ignite();

protected:
  bool collision_squished(GameObject& object);

private:
  GoldBomb(const GoldBomb&);
  GoldBomb& operator=(const GoldBomb&);
  enum Ticking_State {
    STATE_NORMAL,
    STATE_TICKING
  };

  Ticking_State tstate;
  bool grabbed;
  MovingObject* grabber;

  std::unique_ptr<SoundSource> ticking;
};

#endif

/* EOF */
