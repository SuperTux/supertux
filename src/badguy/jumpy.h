#ifndef __JUMPY_H__
#define __JUMPY_H__

#include "badguy.h"
#include "utils/lispreader.h"
#include "utils/lispwriter.h"
#include "serializable.h"

class Jumpy : public BadGuy
{
public:
  Jumpy(LispReader& reader);

  virtual HitResponse collision_solid(GameObject& other,
      const CollisionHit& hit);

  virtual void write(LispWriter& writer);
};

#endif

