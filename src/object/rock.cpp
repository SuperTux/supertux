#include <config.h>

#include "rock.h"
#include "special/sprite.h"
#include "special/sprite_manager.h"
#include "lisp/writer.h"
#include "video/drawing_context.h"
#include "resources.h"
#include "object_factory.h"

Rock::Rock(const lisp::Lisp& reader)
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("rock");
  grabbed = false;
  flags |= FLAG_SOLID;
}

Rock::~Rock()
{
  delete sprite;
}

void
Rock::write(lisp::Writer& writer)
{
  writer.start_list("rock");

  writer.write_float("x", bbox.p1.x);
  writer.write_float("y", bbox.p1.y);

  writer.end_list("rock");
}

void
Rock::draw(DrawingContext& context)
{

  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

void
Rock::action(float elapsed_time)
{
  if(!grabbed) {
    flags |= FLAG_SOLID;
    flags &= ~FLAG_NO_COLLDET;
    movement = physic.get_movement(elapsed_time);
  } else {
    physic.set_velocity(0, 0);
    flags &= ~FLAG_SOLID;
    flags |= FLAG_NO_COLLDET;
  }
  
  grabbed = false;
}

HitResponse
Rock::collision(GameObject& , const CollisionHit& )
{
  if(grabbed)
    return FORCE_MOVE;

  physic.set_velocity(0, 0);
  return CONTINUE;
}

void
Rock::grab(MovingObject& , const Vector& pos)
{
  movement = pos - get_pos();
  grabbed = true;
}

IMPLEMENT_FACTORY(Rock, "rock")

