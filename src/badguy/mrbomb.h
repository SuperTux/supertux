#ifndef __MRBOMB_H__
#define __MRBOMB_H__

#include "badguy.h"

class MrBomb : public BadGuy
{
public:
  MrBomb(LispReader& reader);
  MrBomb(float pos_x, float pos_y, Direction d);

  void activate();
  void write(LispWriter& writer);
  HitResponse collision_solid(GameObject& other, const CollisionHit& hit);

protected:
  bool collision_squished(Player& player);
  bool set_direction;
  Direction initial_direction;  
};

#endif

