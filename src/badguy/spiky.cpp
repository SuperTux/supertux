#include <config.h>

#include "spiky.h"

static const float WALKSPEED = 80;

Spiky::Spiky(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("spiky");
}

void
Spiky::write(lisp::Writer& writer)
{
  writer.start_list("spiky");

  writer.write_float("x", get_pos().x);
  writer.write_float("y", get_pos().y);

  writer.end_list("spiky");
}

void
Spiky::activate()
{
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

HitResponse
Spiky::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // hit floor or roof?
    physic.set_velocity_y(0);
  } else { // hit right or left
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  return CONTINUE;
}

