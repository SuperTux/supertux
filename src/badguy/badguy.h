#ifndef __BADGUY_H__
#define __BADGUY_H__

// moved them here to make it less typing when implementing new badguys
#include <math.h>
#include "timer.h"
#include "special/moving_object.h"
#include "special/sprite.h"
#include "math/physic.h"
#include "object/player.h"
#include "serializable.h"
#include "resources.h"
#include "sector.h"
#include "object_factory.h"
#include "lisp/parser.h"
#include "lisp/lisp.h"
#include "lisp/writer.h"
#include "video/drawing_context.h"
#include "special/sprite_manager.h"

using namespace SuperTux;

class BadGuy : public MovingObject, public Serializable
{
public:
  BadGuy();
  ~BadGuy();

  //virtual void action_activated(float elapsed_time);

  virtual void draw(DrawingContext& context);
  virtual void action(float elapsed_time);
  virtual HitResponse collision(GameObject& other,
      const CollisionHit& hit);

  virtual void kill_fall();

protected:
  enum State {
    STATE_INIT,
    STATE_INACTIVE,
    STATE_ACTIVE,
    STATE_SQUISHED,
    STATE_FALLING
  };
  
  virtual HitResponse collision_player(Player& player,
      const CollisionHit& hit);
  virtual HitResponse collision_solid(GameObject& other,
      const CollisionHit& hit);
  virtual HitResponse collision_badguy(BadGuy& other,
      const CollisionHit& hit);
  
  virtual bool collision_squished(Player& player);

  virtual void active_action(float elapsed_time);
  virtual void inactive_action(float elapsed_time);

  /**
   * called when the badguy has been activated. (As a side effect the dir
   * variable might have been changed so that it faces towards the player.
   */
  virtual void activate();
  /** caleed when the badguy has been deactivated */
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
  Timer2 state_timer;
};

#endif

