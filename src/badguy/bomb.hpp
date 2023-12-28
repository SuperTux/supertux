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

#ifndef HEADER_SUPERTUX_BADGUY_BOMB_HPP
#define HEADER_SUPERTUX_BADGUY_BOMB_HPP

#include "badguy/badguy.hpp"

class SoundSource;

class Bomb final : public BadGuy
{
public:
  Bomb(const Vector& pos, Direction dir, const std::string& custom_sprite = "images/creatures/mr_bomb/bomb.sprite" );
  virtual bool is_saveable() const override {
    return false;
  }

  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision_player(Player& player, const CollisionHit& hit) override;
  virtual HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit) override;

  virtual void active_update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual void kill_fall() override;
  virtual void ignite() override;
  void explode();

  virtual bool is_portable() const override;
  virtual void grab(MovingObject& object, const Vector& pos, Direction dir) override;
  virtual void ungrab(MovingObject& object, Direction dir) override;
  virtual bool is_snipable() const override { return true; }

  virtual void stop_looping_sounds() override;
  virtual void play_looping_sounds() override;

private:
  std::unique_ptr<SoundSource> m_fizz_sound;
  std::unique_ptr<SoundSource> m_ticking_sound;
  SpritePtr m_exploding_sprite;

private:
  Bomb(const Bomb&) = delete;
  Bomb& operator=(const Bomb&) = delete;
};

#endif

/* EOF */
