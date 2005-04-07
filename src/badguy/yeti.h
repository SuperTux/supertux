#ifndef __YETI_H__
#define __YETI_H__

#include "badguy.h"

class Yeti : public BadGuy
{
public:
  Yeti(const lisp::Lisp& lisp);
  ~Yeti();

  void write(lisp::Writer& writer);
  void active_action(float elapsed_time);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);
  HitResponse collision_player(Player& player, const CollisionHit& hit);
  bool collision_squished(Player& player);
  void kill_fall();

private:
  void go_right();
  void go_left();
  void angry_jumping();
  void drop_stalactite();
  
  enum YetiState {
    INIT,
    ANGRY_JUMPING,
    THROW_SNOWBALL,
    GO_RIGHT,
    GO_LEFT
  };
  enum Side {
    LEFT,
    RIGHT
  };
  Side side;
  YetiState state;
  Timer2 timer;
  int jumpcount;
  Mix_Chunk* sound_gna;
  Mix_Chunk* sound_roar;
};

#endif

