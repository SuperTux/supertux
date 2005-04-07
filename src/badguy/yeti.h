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
  HitResponse collision_badguy(BadGuy& badguy, const CollisionHit&);
  bool collision_squished(Player& player);
  void kill_fall();

private:
  void go_right();
  void go_left();
  void angry_jumping();
  void stun();
  void drop_stalactite();
  
  enum YetiState {
    INIT,
    ANGRY_JUMPING,
    THROW_SNOWBALL,
    GO_RIGHT,
    GO_LEFT,
    STUNNED
  };
  enum Side {
    LEFT,
    RIGHT
  };
  Side side;
  YetiState state;
  Timer2 jump_timer;
  Timer2 stun_timer;
  int jumpcount;
  float jump_time_left;
  Mix_Chunk* sound_gna;
  Mix_Chunk* sound_roar;
};

#endif

