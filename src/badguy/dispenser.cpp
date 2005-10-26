//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include <config.h>

#include "dispenser.hpp"
#include "badguy/bouncing_snowball.hpp"
#include "badguy/snowball.hpp"
#include "badguy/mrbomb.hpp"
#include "badguy/mriceblock.hpp"
#include "badguy/mrrocket.hpp"
#include "badguy/poisonivy.hpp"
#include "badguy/snowsnail.hpp"

Dispenser::Dispenser(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  reader.get("cycle", cycle);
  reader.get("badguy", badguy);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("dispenser");
  if (badguy == "mrrocket") {
     sprite->set_action(dir == LEFT ? "working-left" : "working-right");
  }
  else {sprite->set_action("dropper");}
  countMe = false;
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
  sprite->set_action(dir == LEFT ? "broken-left" : "broken-right");
  dispense_timer.start(0);
  player.bounce(*this);
  kill_squished(player);
  return true;
}

void
Dispenser::active_update(float )
{
  if (dispense_timer.check()) {
    launch_badguy();
  }
}

//TODO: Add launching velocity to certain badguys
//      Add themed randomizer
//      Fix initial direction (everyone but MrRocket walks the wrong direction)
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
    else if (badguy == "snowsnail")
      Sector::current()->add_object(new SnowSnail(get_pos().x, get_pos().y+32, dir));
    else if (badguy == "mrrocket") {
      Sector::current()->add_object(new MrRocket(get_pos().x+(dir == LEFT ? -32 : 32), get_pos().y, dir));}
    else if (badguy == "poisonivy")
      Sector::current()->add_object(new PoisonIvy(get_pos().x, get_pos().y+32, dir));
    else if (badguy == "random")
    {
      switch (rand()%6)
      {
        case 0: Sector::current()->add_object(new SnowBall(get_pos().x, get_pos().y+32, dir)); break;
        case 1: Sector::current()->add_object(new BouncingSnowball(get_pos().x, get_pos().y+32, dir)); break;
        case 2: Sector::current()->add_object(new MrBomb(get_pos().x, get_pos().y+32, dir)); break;
        case 3: Sector::current()->add_object(new MrIceBlock(get_pos().x, get_pos().y+32, dir)); break;
        case 4: Sector::current()->add_object(new PoisonIvy(get_pos().x, get_pos().y+32, dir)); break;
        case 5: Sector::current()->add_object(new SnowSnail(get_pos().x, get_pos().y+32, dir)); break;
      }
    }
  }
}

IMPLEMENT_FACTORY(Dispenser, "dispenser")
