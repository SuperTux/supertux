#include <config.h>

#include "dispenser.h"
#include "badguy/bouncing_snowball.h"
#include "badguy/snowball.h"
#include "badguy/mrbomb.h"
#include "badguy/mriceblock.h"
#include "badguy/mrrocket.h"

Dispenser::Dispenser(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  reader.get("cycle", cycle);
  reader.get("badguy", badguy);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("dispenser");
  sprite->set_action("working");
}

void
Dispenser::write(lisp::Writer& writer)
{
  writer.start_list("dispenser");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
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
  //TODO: Should it act like a normal tile when killed?
  sprite->set_action("broken");
  dispense_timer.start(0);
  player.bounce(*this);
  kill_squished(player);
  return true;
}

void
Dispenser::active_action(float )
{
  if (dispense_timer.check()) {
    launch_badguy();
  }
}

//TODO: Add launching velocity to certain badguys
//      Add randomizer (themed to match tileset)
void
Dispenser::launch_badguy()
{
  //FIXME: Does is_offscreen() work right here?
  if (!is_offscreen()) {
    if (badguy == "snowball")
      Sector::current()->add_object(new SnowBall(get_pos().x, get_pos().y+32, dir));
    else if (badguy == "bouncingsnowball")
      Sector::current()->add_object(new BouncingSnowball(get_pos().x, get_pos().y+32, dir));
    else if (badguy == "mrbomb")
      Sector::current()->add_object(new MrBomb(get_pos().x, get_pos().y+32, dir));
    else if (badguy == "mriceblock")
      Sector::current()->add_object(new MrIceBlock(get_pos().x, get_pos().y+32, dir));
    else if (badguy == "mrrocket")
      Sector::current()->add_object(new MrRocket(get_pos().x, get_pos().y+32, dir));
    else if (badguy == "random")
    {}
  }
}

IMPLEMENT_FACTORY(Dispenser, "dispenser")
