#ifndef __STALACTITE_H__
#define __STALACTITE_H__

#include "badguy.h"

class Stalactite : public BadGuy
{
public:
  Stalactite(const lisp::Lisp& reader);
 
  void active_action(float elapsed_time);
  void write(lisp::Writer& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
  HitResponse collision_player(Player& player, const CollisionHit& hit);

  void kill_fall();
  void draw(DrawingContext& context);
  void deactivate();

private:
  Physic physic;
  Timer2 timer;

  enum StalactiteState {
    STALACTITE_HANGING,
    STALACTITE_SHAKING,
    STALACTITE_FALLING,
    STALACTITE_SQUISHED
  };
  StalactiteState state;
};

#endif

