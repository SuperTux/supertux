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

#ifndef HEADER_SUPERTUX_BADGUY_STALACTITE_HPP
#define HEADER_SUPERTUX_BADGUY_STALACTITE_HPP

#include "badguy/badguy.hpp"
#include "video/flip.hpp"

class Stalactite : public BadGuy
{
public:
  Stalactite(const ReaderMapping& reader);

  virtual void active_update(float dt_sec) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision_player(Player& player, const CollisionHit& hit) override;
  virtual HitResponse collision_badguy(BadGuy& other, const CollisionHit& hit) override;
  virtual HitResponse collision_bullet(Bullet& bullet, const CollisionHit& ) override;

  virtual void kill_fall() override;
  virtual void draw(DrawingContext& context) override;
  virtual void deactivate() override;

  virtual std::string get_class() const override { return "stalactite"; }
  virtual std::string get_display_name() const override { return _("Stalactite"); }

  virtual void on_flip(float height) override;

  void squish();

protected:
  enum StalactiteState {
    STALACTITE_HANGING,
    STALACTITE_SHAKING,
    STALACTITE_FALLING,
    STALACTITE_SQUISHED
  };

protected:
  Timer timer;
  StalactiteState state;
  Vector shake_delta;

private:
  Flip m_flip;

private:
  Stalactite(const Stalactite&) = delete;
  Stalactite& operator=(const Stalactite&) = delete;
};

#endif

/* EOF */
