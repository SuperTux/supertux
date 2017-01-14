//  IceCrusher - A block to stand on, which can drop down to crush the player
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_ICECRUSHER_HPP
#define HEADER_SUPERTUX_OBJECT_ICECRUSHER_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"

class Player;

/**
 * This class is the base class for icecrushers that tux can stand on
 */
class IceCrusher : public MovingSprite
{
public:
  IceCrusher(const ReaderMapping& reader);
  IceCrusher(const IceCrusher& icecrusher);

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual void collision_solid(const CollisionHit& hit);
  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);
  std::string get_class() const {
    return "icecrusher";
  }
  std::string get_display_name() const {
    return _("Ice crusher");
  }

#if 0
  const Vector& get_speed() const
  {
    return speed;
  }
#endif

protected:
  enum IceCrusherState {
    IDLE,
    CRUSHING,
    RECOVERING
  };
  IceCrusherState state;
  Vector start_position;
  Physic physic;
  float cooldown_timer;

  bool found_victim() const;
  void set_state(IceCrusherState state, bool force = false);
  Vector eye_position(bool right) const;

  SpritePtr lefteye;
  SpritePtr righteye;
  SpritePtr whites;

private:
  enum IceCrusherSize {
    NORMAL,
    LARGE
  };
  IceCrusherSize ic_size;
};

#endif

/* EOF */
