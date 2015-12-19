//  SuperTux - Weak Block
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_WEAK_BLOCK_HPP
#define HEADER_SUPERTUX_OBJECT_WEAK_BLOCK_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"
#include "object/bullet.hpp"


/**
 * A block that can be destroyed by Bullet hits
 */
class WeakBlock : public MovingSprite
{
public:
  WeakBlock(const Reader& lisp);
  virtual void save(lisp::Writer& writer);

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void update(float elapsed_time);
  void draw(DrawingContext& context);
  virtual std::string get_class() const {
    return "weak_block";
  }

  virtual ObjectSettings get_settings();

protected:
  /**
   * called by self when hit by a bullet
   */
  void startBurning();

  /**
   * pass hit to nearby WeakBlock objects
   */
  void spreadHit();

private:
  enum State {
    STATE_NORMAL, /**< default state */
    STATE_BURNING, /**< on fire, still solid */
    STATE_DISINTEGRATING /**< crumbling to dust, no longer solid */
  };
  State state;
	
  bool linked;
  virtual HitResponse collision_bullet(Bullet& bullet, const CollisionHit& hit);

  Color light;
  SpritePtr lightsprite;

};

#endif

/* EOF */
