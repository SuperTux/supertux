#include <config.h>

#include "poisonivy.h"

static const float WALKSPEED = 80;

PoisonIvy::PoisonIvy(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("poisonivy");
  set_direction = false;
}

PoisonIvy::PoisonIvy(float pos_x, float pos_y, Direction d)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("poisonivy");
  set_direction = true;
  initial_direction = d;
}

void
PoisonIvy::write(lisp::Writer& writer)
{
  writer.start_list("poisonivy");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("poisonivy");
}

void
PoisonIvy::activate()
{
  if (set_direction) {dir = initial_direction;}
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
PoisonIvy::collision_squished(Player& player)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(player);
  return true;
}

HitResponse
PoisonIvy::collision_solid(GameObject& , const CollisionHit& hit)
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

HitResponse
PoisonIvy::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.x) > .8) { // left or right hit
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());       
  }

  return CONTINUE;
}

IMPLEMENT_FACTORY(PoisonIvy, "poisonivy")
