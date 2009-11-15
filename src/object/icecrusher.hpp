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

#include <memory>
#include <string>
#include "object/moving_sprite.hpp"
#include "object/player.hpp"

/**
 * This class is the base class for icecrushers that tux can stand on
 */
class IceCrusher : public MovingSprite
{
  public:
    IceCrusher(const lisp::Lisp& reader);
    IceCrusher(const IceCrusher& icecrusher);
    virtual IceCrusher* clone() const { return new IceCrusher(*this); }

    virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
    virtual void collision_solid(const CollisionHit& hit);
    virtual void update(float elapsed_time);

    const Vector& get_speed() const
    {
      return speed;
    }

  protected:
    enum IceCrusherState {
      IDLE,
      CRUSHING,
      RECOVERING
    };
    IceCrusherState state;
    Vector start_position;
    Vector speed;

    Player* get_nearest_player();
    bool found_victim();
    void set_state(IceCrusherState state, bool force = false);

};

#endif
