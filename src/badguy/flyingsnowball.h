#ifndef __FLYINGSNOWBALL_H__
#define __FLYINGSNOWBALL_H__

#include "badguy.h"

class FlyingSnowBall : public BadGuy
{
public:
  FlyingSnowBall(const lisp::Lisp& reader);
  FlyingSnowBall(float pos_x, float pos_y);

  void activate();
  void write(lisp::Writer& writer);
  void active_action(float elapsed_time);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
protected:
  enum FlyingSnowballMode {
    FLY_UP,
    FLY_DOWN
  };
  FlyingSnowballMode mode;
  bool collision_squished(Player& player);
private:
  Timer2 timer;
};

#endif

