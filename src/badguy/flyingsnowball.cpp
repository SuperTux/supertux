#include <config.h>
#include <stdio.h>

#include "flyingsnowball.h"

static const float FLYTIME = 1.0;
static const float FLYSPEED = 100.0;

FlyingSnowBall::FlyingSnowBall(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("flyingsnowball");
  physic.enable_gravity(false);
}

FlyingSnowBall::FlyingSnowBall(float pos_x, float pos_y)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("flyingsnowball");
  physic.enable_gravity(false);
}

void
FlyingSnowBall::write(lisp::Writer& writer)
{
  writer.start_list("flyingsnowball");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("flyingsnowball");
}

void 
FlyingSnowBall::activate()
{
  sprite->set_action(dir == LEFT ? "left" : "right");
  mode = FLY_UP;
  physic.set_velocity_y(FLYSPEED);
  timer.start(FLYTIME/2);
}

bool
FlyingSnowBall::collision_squished(Player& player)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(player);
  return true;
}

HitResponse
FlyingSnowBall::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // hit floor or roof?
    physic.set_velocity_y(0);
  }

  return CONTINUE;
}

void
FlyingSnowBall::active_action(float elapsed_time) 
{
  if(timer.check()) {
    if(mode == FLY_UP) {
      mode = FLY_DOWN;
      physic.set_velocity_y(-FLYSPEED);
    } else if(mode == FLY_DOWN) {
      mode = FLY_UP;
      physic.set_velocity_y(FLYSPEED);
    }
    timer.start(FLYTIME);
  }
  movement=physic.get_movement(elapsed_time);
  dir= Sector::current()->player->get_pos().x>get_pos().x?RIGHT:LEFT;
  sprite->set_action(dir == LEFT ? "left" : "right");
}

IMPLEMENT_FACTORY(FlyingSnowBall, "flyingsnowball")
