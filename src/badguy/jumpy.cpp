#include <config.h>

#include "jumpy.h"

static const float JUMPSPEED=600;

Jumpy::Jumpy(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("jumpy");
}

void
Jumpy::write(lisp::Writer& writer)
{
  writer.start_list("jumpy");

  writer.write_float("x", get_pos().x);
  writer.write_float("y", get_pos().y);

  writer.end_list("jumpy");
}

HitResponse
Jumpy::collision_solid(GameObject& , const CollisionHit& hit)
{
  // hit floor?
  if(hit.normal.y < -.5) {
    physic.set_velocity_y(JUMPSPEED);
  } else if(hit.normal.y < .5) { // bumped on roof
    physic.set_velocity_y(0);
  }

  return CONTINUE;
}

