#include <config.h>

#include "dispenser.h"
#include "badguy/bouncing_snowball.h"
#include "badguy/snowball.h"
#include "badguy/mrbomb.h"
#include "badguy/mriceblock.h"


Dispenser::Dispenser(LispReader& reader)
{
  reader.read_float("x", start_position.x);
  reader.read_float("y", start_position.y);
  reader.read_float("cycle", cycle);
  reader.read_string("badguy", badguy);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("dispenser");
  sprite->set_action("working");
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
   launch_badguy();
}

bool
Dispenser::collision_squished(Player& player)
{
  //FIXME: Should act like a normal tile when killed
  sprite->set_action("broken");
  dispense_timer.start(0);
  player.bounce(*this);
  return true;
}

void
Dispenser::active_action(float )
{
  if (dispense_timer.check()) {
    launch_badguy();
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

//TODO: Add launching velocity to badguys
//      Add randomizer
//      Clean up stuff I copied without understanding what it does :)
//      Stop dispensing when game is paused (timer related problem)
//      Lots-O-Stuff (tm)
void
Dispenser::launch_badguy()
{
  //FIXME: Does is_offscreen() work right here?
  if (!is_offscreen()) {
    if (badguy == "snowball")
      Sector::current()->add_object(new SnowBall(get_pos().x, get_pos().y, dir));
    else if (badguy == "bouncingsnowball")
      Sector::current()->add_object(new BouncingSnowball(get_pos().x, get_pos().y, dir));
    else if (badguy == "mrbomb")
      Sector::current()->add_object(new MrBomb(get_pos().x, get_pos().y, dir));
    else if (badguy == "mriceblock")
      Sector::current()->add_object(new MrIceBlock(get_pos().x, get_pos().y, dir));
    else if (badguy == "random")
    {}
  }
}

