#include <config.h>

#include "bouncing_snowball.h"

static const float JUMPSPEED = 450;
static const float WALKSPEED = 80;

BouncingSnowball::BouncingSnowball(LispReader& reader)
{
  reader.read_float("x", start_position.x);
  reader.read_float("y", start_position.y);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("bouncingsnowball");
}

BouncingSnowball::BouncingSnowball(float pos_x, float pos_y)
{
   start_position.x = pos_x;
   start_position.y = pos_y;
   bbox.set_size(32, 32);
   sprite = sprite_manager->create("bouncingsnowball");
}

void
BouncingSnowball::write(LispWriter& writer)
{
  writer.start_list("bouncingsnowball");

  writer.write_float("x", get_pos().x);
  writer.write_float("y", get_pos().y);

  writer.end_list("bouncingsnowball");
}

void
BouncingSnowball::activate()
{
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

