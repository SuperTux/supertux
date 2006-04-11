//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#ifndef __BADGUY_H__
#define __BADGUY_H__

// moved them here to make it less typing when implementing new badguys
#include <math.h>
#include "timer.hpp"
#include "moving_object.hpp"
#include "sprite/sprite.hpp"
#include "physic.hpp"
#include "object/player.hpp"
#include "serializable.hpp"
#include "resources.hpp"
#include "sector.hpp"
#include "direction.hpp"
#include "object_factory.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"
#include "video/drawing_context.hpp"
#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "sprite/sprite_manager.hpp"

class BadGuy : public MovingObject, public Serializable
{
public:
  BadGuy();
  ~BadGuy();

  /** Called when the badguy is drawn. The default implementation simply draws
   * the badguy sprite on screen
   */
  virtual void draw(DrawingContext& context);
  /** Called each frame. The default implementation checks badguy state and
   * calls active_update and inactive_update
   */
  virtual void update(float elapsed_time);
  /** Called when a collision with another object occured. The default
   * implemetnation calls collision_player, collision_solid, collision_badguy
   * and collision_squished
   */
  virtual HitResponse collision(GameObject& other,
      const CollisionHit& hit);

  /** Called when a collision with tile with special attributes occured */
  virtual void collision_tile(uint32_t tile_attributes);

  /** Set the badguy to kill/falling state, which makes him falling of the
   * screen (his sprite is turned upside-down)
   */
  virtual void kill_fall();

  /** Writes out the badguy into the included lisp::Writer. Useful e.g. when
   * converting an old-format level to the new format.
   */
  virtual void save(lisp::Writer& writer);

  Vector get_start_position() const
  {
    return start_position;
  }
  void set_start_position(const Vector& vec)
  {
    start_position = vec;
  }
  
  /** Count this badguy to the statistics? This value should not be changed
   * during runtime. */
  bool countMe;

protected:
  enum State {
    STATE_INIT,
    STATE_INACTIVE,
    STATE_ACTIVE,
    STATE_SQUISHED,
    STATE_FALLING
  };
 
  /** Called when the badguy collided with a player */
  virtual HitResponse collision_player(Player& player,
      const CollisionHit& hit);
  /** Called when the badguy collided with solid ground */
  virtual HitResponse collision_solid(GameObject& other,
      const CollisionHit& hit);
  /** Called when the badguy collided with another badguy */
  virtual HitResponse collision_badguy(BadGuy& other,
      const CollisionHit& hit);
 
  /** Called when the player hit the badguy from above. You should return true
   * if the badguy was squished, false if squishing wasn't possible
   */
  virtual bool collision_squished(Player& player);

  /** called each frame when the badguy is activated. */
  virtual void active_update(float elapsed_time);
  /** called each frame when the badguy is not activated. */
  virtual void inactive_update(float elapsed_time);

  /**
   * called when the badguy has been activated. (As a side effect the dir
   * variable might have been changed so that it faces towards the player.
   */
  virtual void activate();
  /** called when the badguy has been deactivated */
  virtual void deactivate();

  void kill_squished(Player& player);

  void set_state(State state);
  State get_state() const
  { return state; }
    
  /**
   * returns a pointer to the nearest player or 0 if no player is available
   */
  Player* get_nearest_player();
  
  Sprite* sprite;
  Physic physic;

  /// is the enemy activated
  bool activated;
  /**
   * Should the badguy be removed when it leaves the bounds of the sector?
   * Use this when the badguy is portable and picked up by the player, who
   * may actually leave the sector's bounds sometimes.
   */
  bool remove_out_of_bounds;
  /**
   * initial position of the enemy. Also the position where enemy respawns when
   * after being deactivated.
   */
  bool is_offscreen();
  /**
   * Checks if the badguy may fall off a platform if continuing a given movement.
   */
  bool may_fall_off_platform();

  Vector start_position;

  Direction dir;
private:
  void try_activate();
  
  State state;
  Timer state_timer;
};

#endif

