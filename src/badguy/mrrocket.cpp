#include <config.h>

#include "mrrocket.h"

static const float SPEED = 200;

MrRocket::MrRocket(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("mrrocket");
  set_direction = false;
}

MrRocket::MrRocket(float pos_x, float pos_y, Direction d)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("mrrocket");
  set_direction = true;
  initial_direction = d;
}

void
MrRocket::write(lisp::Writer& writer)
{
  writer.start_list("mrrocket");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("mrrocket");
}

void
MrRocket::activate()
{
  if (set_direction) {dir = initial_direction;}
  physic.set_velocity_x(dir == LEFT ? -SPEED : SPEED);
  physic.enable_gravity(false);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

void
MrRocket::active_action(float elapsed_time)
{
  if (collision_timer.check()) {
    Sector::current()->add_object(new RocketExplosion(get_pos(), dir));
    remove_me();
  }
  else if (!collision_timer.started()) {
     movement=physic.get_movement(elapsed_time);
     sprite->set_action(dir == LEFT ? "left" : "right");
  }
}

bool
MrRocket::collision_squished(Player& player)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(player);
  kill_fall();
  return true;
}

HitResponse
MrRocket::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // hit floor or roof?
    physic.set_velocity_y(0);
  } else { // hit right or left
    sprite->set_action(dir == LEFT ? "collision-left" : "collision-right");
    physic.set_velocity_x(0);
    collision_timer.start(0.2, true);
  }

  return CONTINUE;
}

IMPLEMENT_FACTORY(MrRocket, "mrrocket")
