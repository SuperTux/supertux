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

#include "audio/sound_source.hpp"
#include "badguy/badguy.hpp"
#include "object/portable.hpp"

class Bomb : public BadGuy,
             public Portable
{
public:
  Bomb(const Vector& pos, Direction dir, std::string custom_sprite = "images/creatures/mr_bomb/bomb.sprite" );
  virtual bool do_save() {
    return false;
  }

  void collision_solid(const CollisionHit& hit);
  HitResponse collision_player(Player& player, const CollisionHit& hit);
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit& hit);
  void active_update(float elapsed_time);
  void kill_fall();
  void ignite();
  void explode();
  void grab(MovingObject& object, const Vector& pos, Direction dir);
  void ungrab(MovingObject& object, Direction dir);

private:
  enum State {
    STATE_TICKING
  };

  State state;
  bool grabbed;
  MovingObject* grabber;

  std::unique_ptr<SoundSource> ticking;

private:
  Bomb(const Bomb&);
  Bomb& operator=(const Bomb&);
};

#endif

/* EOF */
