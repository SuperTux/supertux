#ifndef __MRICEBLOCK_H__
#define __MRICEBLOCK_H__

#include "badguy.h"

class MrIceBlock : public BadGuy
{
public:
  MrIceBlock(const lisp::Lisp& reader);
  MrIceBlock(float pos_x, float pos_y, Direction d);

  void activate();
  void write(lisp::Writer& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);

  void active_action(float elapsed_time);

protected:
  bool collision_squished(Player& player);

private:
  enum IceState {
    ICESTATE_NORMAL,
    ICESTATE_FLAT,
    ICESTATE_KICKED
  };
  IceState ice_state;
  Timer2 flat_timer;
  int squishcount;
  bool set_direction;
  Direction initial_direction;  
};

#endif

