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
#include "object/bullet.hpp"
#include "badguy/bouncing_snowball.hpp"
#include "badguy/snowball.hpp"
#include "badguy/mrbomb.hpp"
#include "badguy/mriceblock.hpp"
#include "badguy/mrrocket.hpp"
#include "badguy/poisonivy.hpp"
#include "badguy/snail.hpp"
#include "badguy/skullyhop.hpp"
#include "badguy/captainsnowball.hpp"
#include "badguy/kamikazesnowball.hpp"
#include "random_generator.hpp"

Dispenser::Dispenser(const lisp::Lisp& reader)
	: BadGuy(reader, "images/creatures/dispenser/dispenser.sprite")
{
  set_colgroup_active(COLGROUP_MOVING_STATIC);
  sound_manager->preload("sounds/squish.wav");
  reader.get("cycle", cycle);
  reader.get("badguy", badguy);
  autotarget = false;
  swivel = false;
  broken = false;
  if (badguy == "mrrocket") {
     sprite->set_action(dir == LEFT ? "working-left" : "working-right");
     set_colgroup_active(COLGROUP_MOVING); //if this were COLGROUP_MOVING_STATIC MrRocket would explode on launch.
     if( start_dir == AUTO ){
      autotarget = true;
     }
  } else if ( badguy == "kamikazesnowball" ||  badguy == "captainsnowball" ) {
     sprite->set_action("working");
  } else {
    sprite->set_action("dropper");
  }
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
   if( broken ){
     return;
   }
   if( autotarget && !swivel ){ // auto cannon sprite might be wrong
      Player* player = this->get_nearest_player();
      if( player ){
        dir = (player->get_pos().x > get_pos().x) ? RIGHT : LEFT;
        sprite->set_action(dir == LEFT ? "working-left" : "working-right");
      }
   }
   dispense_timer.start(cycle, true);
   launch_badguy();
}

void
Dispenser::deactivate()
{
   dispense_timer.stop();
}

//TODO: Add launching velocity to certain badguys
bool
Dispenser::collision_squished(GameObject& object)
{
  //Cannon launching MrRocket can be broken by jumping on it
  //other dispencers are not that fragile.
  if (broken || badguy != "mrrocket") {
    return false;
  }
  sprite->set_action(dir == LEFT ? "broken-left" : "broken-right");
  dispense_timer.start(0);
  set_colgroup_active(COLGROUP_MOVING_STATIC); // Tux can stand on broken cannon.
  Player* player = dynamic_cast<Player*>(&object);
  if (player){
    player->bounce(*this);
  }
  sound_manager->play("sounds/squish.wav", get_pos());
  broken = true;
  return true;
}

HitResponse
Dispenser::collision(GameObject& other, const CollisionHit& hit)
{
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    // hit from above?
    if (player->get_bbox().p2.y < (bbox.p1.y + 16)) {
      collision_squished(*player);
      return FORCE_MOVE;
    }
    if(frozen){
      unfreeze();
    }
    return FORCE_MOVE;
  }

  Bullet* bullet = dynamic_cast<Bullet*> (&other);
  if(bullet){
    return collision_bullet(*bullet, hit);
  }

  return FORCE_MOVE;
}


void
Dispenser::active_update(float )
{
  if (dispense_timer.check()) {
    // auto always shoots in Tux's direction
    if( autotarget ){ 
      if( sprite->animation_done()) {
        sprite->set_action(dir == LEFT ? "working-left" : "working-right");
        swivel = false;
      }

      Player* player = this->get_nearest_player();
      if( player && !swivel ){
        Direction targetdir = (player->get_pos().x > get_pos().x) ? RIGHT : LEFT;
        if( dir != targetdir ){ // no target: swivel cannon 
          swivel = true;
          dir = targetdir;
          sprite->set_action(dir == LEFT ? "swivel-left" : "swivel-right", 1);
        } else { // tux in sight: shoot
          launch_badguy();
        }
      }
    } else {
      launch_badguy();
    }
  }
}

//      Add themed randomizer
void
Dispenser::launch_badguy()
{
  //FIXME: Does is_offscreen() work right here?
  if (!is_offscreen()) {
    Direction launchdir = dir;
    if( !autotarget && start_dir == AUTO ){
      Player* player = this->get_nearest_player();
      if( player ){
        launchdir = (player->get_pos().x > get_pos().x) ? RIGHT : LEFT;
      } 
    } 
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
    else if (badguy == "mrrocket") 
      Sector::current()->add_object(new MrRocket(Vector(get_pos().x+(launchdir == LEFT ? -32 : 32), get_pos().y), launchdir));
    else if (badguy == "captainsnowball")
      Sector::current()->add_object(new CaptainSnowball(Vector(get_pos().x+(launchdir == LEFT ? -32 : 32), get_pos().y), launchdir));
    else if (badguy == "kamikazesnowball")
      Sector::current()->add_object(new KamikazeSnowball(Vector(get_pos().x+(launchdir == LEFT ? -32 : 32), get_pos().y), launchdir));
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

void
Dispenser::freeze()
{
  BadGuy::freeze();
  dispense_timer.stop();
}

void
Dispenser::unfreeze()
{
  BadGuy::unfreeze();
  activate();
}

bool
Dispenser::is_freezable() const
{
  return true;
}
IMPLEMENT_FACTORY(Dispenser, "dispenser")
