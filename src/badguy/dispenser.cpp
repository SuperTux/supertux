//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include "dispenser.hpp"
#include "badguy/bouncing_snowball.hpp"
#include "badguy/snowball.hpp"
#include "badguy/mrbomb.hpp"
#include "badguy/mriceblock.hpp"
#include "badguy/mrrocket.hpp"
#include "badguy/poisonivy.hpp"
#include "badguy/snail.hpp"
#include "badguy/skullyhop.hpp"
#include "random_generator.hpp"

Dispenser::Dispenser(const lisp::Lisp& reader)
	: BadGuy(reader, "images/creatures/dispenser/dispenser.sprite")
{
  std::string launchdirection = "";
  launchdir = dir;
  reader.get("launchdirection", launchdirection);
  if( launchdirection == "left" || launchdirection == "LEFT"  )
    launchdir = LEFT;
  if( launchdirection == "right" || launchdirection == "RIGHT"  ) 
    launchdir = RIGHT;
  reader.get("cycle", cycle);
  reader.get("badguy", badguy);
  if (badguy == "mrrocket") {
     sprite->set_action(launchdir == LEFT ? "working-left" : "working-right");
  }
  else {sprite->set_action("dropper");}
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
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
  sprite->set_action(launchdir == LEFT ? "broken-left" : "broken-right");
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
void
Dispenser::launch_badguy()
{
  //FIXME: Does is_offscreen() work right here?
  if (!is_offscreen()) {
    if (badguy == "snowball")
      Sector::current()->add_object(new SnowBall(Vector(get_pos().x, get_pos().y+32), launchdir));
    else if (badguy == "bouncingsnowball")
      Sector::current()->add_object(new BouncingSnowball(Vector(get_pos().x, get_pos().y+32), launchdir));
    else if (badguy == "mrbomb")
      Sector::current()->add_object(new MrBomb(Vector(get_pos().x, get_pos().y+32), launchdir));
    else if (badguy == "mriceblock")
      Sector::current()->add_object(new MrIceBlock(Vector(get_pos().x, get_pos().y+32), launchdir));
    else if (badguy == "snail")
      Sector::current()->add_object(new Snail(Vector(get_pos().x, get_pos().y+32), launchdir));
    else if (badguy == "mrrocket") {
      Sector::current()->add_object(new MrRocket(Vector(get_pos().x+(launchdir == LEFT ? -32 : 32), get_pos().y), launchdir));}
    else if (badguy == "poisonivy")
      Sector::current()->add_object(new PoisonIvy(Vector(get_pos().x, get_pos().y+32), launchdir));
    else if (badguy == "skullyhop")
      Sector::current()->add_object(new SkullyHop(Vector(get_pos().x, get_pos().y+44), launchdir));
    else if (badguy == "random")
    {
      switch (systemRandom.rand(7))
      {
        case 0: Sector::current()->add_object(new SnowBall(Vector(get_pos().x, get_pos().y+32), launchdir)); break;
        case 1: Sector::current()->add_object(new BouncingSnowball(Vector(get_pos().x, get_pos().y+32), launchdir)); break;
        case 2: Sector::current()->add_object(new MrBomb(Vector(get_pos().x, get_pos().y+32), launchdir)); break;
        case 3: Sector::current()->add_object(new MrIceBlock(Vector(get_pos().x, get_pos().y+32), launchdir)); break;
        case 4: Sector::current()->add_object(new PoisonIvy(Vector(get_pos().x, get_pos().y+32), launchdir)); break;
        case 5: Sector::current()->add_object(new Snail(Vector(get_pos().x, get_pos().y+32), launchdir)); break;
        case 6: Sector::current()->add_object(new SkullyHop(Vector(get_pos().x, get_pos().y+44), launchdir)); break;
      }
    }
  }
}

IMPLEMENT_FACTORY(Dispenser, "dispenser")
