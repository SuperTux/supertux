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

#include "audio/fwd.hpp"
#include "badguy/walking_badguy.hpp"

class GoldBomb final : public WalkingBadguy
{
public:
  GoldBomb(const ReaderMapping& reader);

  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision(GameObject& object, const CollisionHit& hit) override;
  virtual HitResponse collision_player(Player& player, const CollisionHit& hit) override;
  virtual HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit) override;

  virtual void active_update(float dt_sec) override;

  virtual void grab(MovingObject& object, const Vector& pos, Direction dir) override;
  virtual void ungrab(MovingObject& object, Direction dir) override;
  virtual bool is_portable() const override;

  virtual void freeze() override;
  virtual bool is_freezable() const override;

  virtual void kill_fall() override;
  virtual void ignite() override;
  virtual std::string get_class() const override { return "goldbomb"; }
  virtual std::string get_display_name() const override { return _("Gold Bomb"); }

  virtual void stop_looping_sounds() override;
  virtual void play_looping_sounds() override;

protected:
  virtual bool collision_squished(GameObject& object) override;

private:
  enum Ticking_State {
    STATE_NORMAL,
    STATE_TICKING
  };

private:
  Ticking_State tstate;

  std::unique_ptr<SoundSource> ticking;

private:
  GoldBomb(const GoldBomb&) = delete;
  GoldBomb& operator=(const GoldBomb&) = delete;
};

#endif

/* EOF */
