#include <config.h>

#include <math.h>
#include "specialriser.h"
#include "resources.h"
#include "camera.h"
#include "sector.h"
#include "app/globals.h"
#include "special/sprite_manager.h"

SpecialRiser::SpecialRiser(MovingObject* _child)
  : child(_child)
{
  offset = 0;
}

SpecialRiser::~SpecialRiser()
{
}

void
SpecialRiser::action(float elapsed_time)
{
  offset += 50 * elapsed_time;
  if(offset > 32) {
    Sector::current()->add_object(child);
    remove_me();
  }
}

void
SpecialRiser::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(
      context.get_translation() + Vector(0, -32 + offset));
  child->draw(context);
  context.pop_transform();
}

