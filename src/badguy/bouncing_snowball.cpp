#include <config.h>

#include "bouncing_snowball.h"

static const float JUMPSPEED = 450;
static const float WALKSPEED = 80;

BouncingSnowball::BouncingSnowball(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("bouncingsnowball");
  set_direction = false;
}

BouncingSnowball::BouncingSnowball(float pos_x, float pos_y, Direction d)
{
   start_position.x = pos_x;
   start_position.y = pos_y;
   bbox.set_size(31.8, 31.8);
   sprite = sprite_manager->create("bouncingsnowball");
   set_direction = true;
   initial_direction = d;
}

void
BouncingSnowball::write(lisp::Writer& writer)
{
  writer.start_list("bouncingsnowball");

  writer.write_float("x", get_pos().x);
  writer.write_float("y", get_pos().y);

  writer.end_list("bouncingsnowball");
}

void
BouncingSnowball::activate()
{
  if (set_direction) {dir = initial_direction;}
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
BouncingSnowball::collision_squished(Player& player)
{
  sprite->set_action("squished");
  kill_squished(player);
  return true;
}

HitResponse
BouncingSnowball::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(hit.normal.y < -.5) { // hit floor
    physic.set_velocity_y(JUMPSPEED);
  } else if(hit.normal.y > .5) { // bumped on roof
    physic.set_velocity_y(0);
  } else { // left or right collision
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  return CONTINUE;
}

HitResponse
BouncingSnowball::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.x) > .8) { // left/right?
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");    
    physic.set_velocity_x(-physic.get_velocity_x());
  } else if(hit.normal.y < -.8) { // grounf
    physic.set_velocity_y(JUMPSPEED);
  }

  return CONTINUE;
}

