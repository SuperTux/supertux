#include <config.h>

#include "nolok_01.h"
#include "badguy/bouncing_snowball.h"
#include "trigger/door.h"

#define WALK_TIME 2.5
#define SHOOT_TIME 0.4
#define JUMP_TIME 0.3

static const float WALKSPEED = 90;

//TODO: Create sprite, give multiple hitpoints, limit max number of snowballs
//      Can only be killed when jumping, no idea why
//      Stop actions when pause button is hit (probably a general problem of timers)
Nolok_01::Nolok_01(LispReader& reader)
{
  reader.read_float("x", start_position.x);
  reader.read_float("y", start_position.y);
  bbox.set_size(31.8, 63.8);
  sprite = sprite_manager->create("dummyguy");
}

Nolok_01::Nolok_01(float pos_x, float pos_y)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  bbox.set_size(31.8, 63.8);
  sprite = sprite_manager->create("dummyguy");
}

void
Nolok_01::write(LispWriter& writer)
{
  writer.start_list("nolok01");

  writer.write_float("x", get_pos().x);
  writer.write_float("y", get_pos().y);

  writer.end_list("nolok01");
}

void
Nolok_01::activate()
{
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
  action = WALKING;
  action_timer.start(WALK_TIME);
}

void
Nolok_01::active_action(float elapsed_time)
{
   movement = physic.get_movement(elapsed_time);
   if (action_timer.check()) {
     if (action == WALKING) {
        physic.set_velocity_y(700);
        action = JUMPING;
        action_timer.start(JUMP_TIME);
     }
     else if (action == JUMPING) {
        sprite->set_action("throw");
        action = SHOOTING;
        action_timer.start(SHOOT_TIME);
     }
     else if (action == SHOOTING) {
        Sector::current()->add_object(new BouncingSnowball(get_pos().x - 64, get_pos().y, LEFT));
        Sector::current()->add_object(new BouncingSnowball(get_pos().x + 64, get_pos().y, RIGHT));
        physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
        sprite->set_action(dir == LEFT ? "left" : "right");
        action = WALKING;
        action_timer.start(WALK_TIME);
     }
   }
}

bool
Nolok_01::collision_squished(Player& player)
{
  sprite->set_action("dead"); 
  kill_squished(player);
  Sector::current()->add_object(new Door((int)get_pos().x+32, 512, "sector1", "main2"));
  return true;
}

HitResponse
Nolok_01::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5){ // hit floor or roof?
    if (action != JUMPING) physic.set_velocity_y(0);
  } else { // hit right or left
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  return CONTINUE;
}

