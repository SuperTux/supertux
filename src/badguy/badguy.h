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
#include "timer.h"
#include "moving_object.h"
#include "sprite/sprite.h"
#include "physic.h"
#include "object/player.h"
#include "serializable.h"
#include "resources.h"
#include "sector.h"
#include "direction.h"
#include "object_factory.h"
#include "lisp/parser.h"
#include "lisp/lisp.h"
#include "lisp/writer.h"
#include "video/drawing_context.h"
#include "sprite/sprite_manager.h"

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

  /** Set the badguy to kill/falling state, which makes him falling of the
   * screen (his sprite is turned upside-down)
   */
  virtual void kill_fall();

  Vector get_start_position() const
  {
    return start_position;
  }
  void set_start_position(const Vector& vec)
  {
    start_position = vec;
  }

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
   * returns a pointer to the player, try to avoid this function to avoid
   * problems later when we have multiple players or no player in scripted
   * sequence.
   */
  Player* get_player();
  
  Sprite* sprite;
  Physic physic;

  /// is the enemy activated
  bool activated;
  /**
   * initial position of the enemy. Also the position where enemy respawns when
   * after being deactivated.
   */
  bool is_offscreen();
  
  Vector start_position;

  Direction dir;
private:
  void try_activate();
  
  State state;
  Timer state_timer;
};

#endif

