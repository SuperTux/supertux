#include <config.h>

#include "dispenser.h"
#include "badguy/bouncing_snowball.h"

Dispenser::Dispenser(LispReader& reader)
{
  reader.read_float("x", start_position.x);
  reader.read_float("y", start_position.y);
  reader.read_float("cycle", cycle);
  reader.read_string("badguy", badguy);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("snowball");
}

void
Dispenser::write(LispWriter& writer)
{
  writer.start_list("dispenser");

  writer.write_float("x", get_pos().x);
  writer.write_float("y", get_pos().y);
  writer.write_float("cycle", cycle);
  writer.write_string("badguy", badguy);

  writer.end_list("dispenser");
}

void
Dispenser::activate()
{
   dispense_timer.start(cycle, true);
}

bool
Dispenser::collision_squished(Player& player)
{
  remove_me();
  player.bounce(*this);
  return true;
}

void
Dispenser::active_action(float elapsed_time)
{
   if (dispense_timer.check()) {
      Sector::current()->add_object(new BouncingSnowball(get_pos().x, get_pos().y));
   }
}

HitResponse
Dispenser::collision_solid(GameObject& , const CollisionHit& hit)
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
