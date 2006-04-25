//  $Id$
// 
//  SuperTux - Boss "Yeti"
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include <float.h>
#include <sstream>
#include <memory>
#include "yeti.hpp"
#include "object/camera.hpp"
#include "yeti_stalactite.hpp"
#include "bouncing_snowball.hpp"
#include "game_session.hpp"

namespace {
  const float JUMP_DOWN_VX = 250; /**< horizontal speed while jumping off the dais */
  const float JUMP_DOWN_VY = 250; /**< vertical speed while jumping off the dais */

  const float RUN_VX = 350; /**< horizontal speed while running */

  const float JUMP_UP_VX = 350; /**< horizontal speed while jumping on the dais */
  const float JUMP_UP_VY = 800; /**< vertical speed while jumping on the dais */

  const float STOMP_VY = 250; /** vertical speed while stomping on the dais */

  const float LEFT_STAND_X = 16; /**< x-coordinate of left dais' end position */
  const float RIGHT_STAND_X = 800-60-16; /**< x-coordinate of right dais' end position */ 
  const float LEFT_JUMP_X = LEFT_STAND_X+224; /**< x-coordinate of from where to jump on the left dais */
  const float RIGHT_JUMP_X = RIGHT_STAND_X-224; /**< x-coordinate of from where to jump on the right dais */
  const float STOMP_WAIT = .5; /**< time we stay on the dais before jumping again */
  const float SAFE_TIME = .5; /**< the time we are safe when tux just hit us */
  const int INITIAL_HITPOINTS = 3; /**< number of hits we can take */
}

Yeti::Yeti(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(60, 90);
  sprite = sprite_manager->create("images/creatures/yeti/yeti.sprite");
  hit_points = INITIAL_HITPOINTS;
  reader.get("dead-script", dead_script);
  countMe = false;
}

Yeti::~Yeti()
{
}

void
Yeti::activate()
{
  dir = RIGHT;
  jump_down();
}

void
Yeti::draw(DrawingContext& context)
{
  // we blink when we are safe
  if(safe_timer.started() && size_t(game_time*40)%2)
    return;

  BadGuy::draw(context);
}

void
Yeti::active_update(float elapsed_time)
{
  switch(state) {
    case JUMP_DOWN:
      physic.set_velocity_x((dir==RIGHT)?+JUMP_DOWN_VX:-JUMP_DOWN_VX);
      break;
    case RUN:
      physic.set_velocity_x((dir==RIGHT)?+RUN_VX:-RUN_VX);
      if (((dir == RIGHT) && (get_pos().x >= RIGHT_JUMP_X)) ||
          ((dir == LEFT) && (get_pos().x <= LEFT_JUMP_X)))
            jump_up();
      break;
    case JUMP_UP:
      physic.set_velocity_x((dir==RIGHT)?+JUMP_UP_VX:-JUMP_UP_VX);
      if (((dir == RIGHT) && (get_pos().x >= RIGHT_STAND_X)) ||
          ((dir == LEFT) && (get_pos().x <= LEFT_STAND_X)))
            be_angry();
      break;
    case BE_ANGRY:
      if(stomp_timer.check()) {
        sound_manager->play("sounds/yeti_gna.wav");
        physic.set_velocity_y(STOMP_VY);
        sprite->set_action((dir==RIGHT)?"stomp-right":"stomp-left");
      }
      break;
  }

  movement = physic.get_movement(elapsed_time);
}

void
Yeti::jump_down()
{
  sprite->set_action((dir==RIGHT)?"jump-right":"jump-left");
  physic.set_velocity_x((dir==RIGHT)?(+JUMP_DOWN_VX):(-JUMP_DOWN_VX));
  physic.set_velocity_y(JUMP_DOWN_VY);
  state = JUMP_DOWN;
}

void
Yeti::run()
{
  sprite->set_action((dir==RIGHT)?"run-right":"run-left");
  physic.set_velocity_x((dir==RIGHT)?(+RUN_VX):(-RUN_VX));
  physic.set_velocity_y(0);
  state = RUN;
}

void
Yeti::jump_up()
{
  sprite->set_action((dir==RIGHT)?"jump-right":"jump-left");
  physic.set_velocity_x((dir==RIGHT)?(+JUMP_UP_VX):(-JUMP_UP_VX));
  physic.set_velocity_y(JUMP_UP_VY);
  state = JUMP_UP;
}

void
Yeti::be_angry()
{
  //turn around
  dir = (dir==RIGHT)?LEFT:RIGHT;

  sprite->set_action((dir==RIGHT)?"stand-right":"stand-left");
  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
  state = BE_ANGRY;
  if (hit_points < INITIAL_HITPOINTS) summon_snowball();
  stomp_count = 0;
  stomp_timer.start(STOMP_WAIT);
}

void
Yeti::die(Player& player)
{
  sprite->set_action("dead", 1);
  kill_squished(player);

  // start script
  if(dead_script != "") {
    std::istringstream stream(dead_script);
    Sector::current()->run_script(stream, "Yeti - dead-script");
  }
}

void
Yeti::summon_snowball()
{
  Sector::current()->add_object(new BouncingSnowball(get_pos().x+(dir == RIGHT ? 64 : -64), get_pos().y, dir));
}

bool
Yeti::collision_squished(Player& player)
{
  player.bounce(*this);

  return false;
}

void
Yeti::take_hit(Player& player)
{
  if (safe_timer.started())
    return;

  sound_manager->play("sounds/yeti_roar.wav");
  hit_points--;

  if(hit_points <= 0 && !player.is_dead()) {
    die(player);
  }
  else {
    safe_timer.start(SAFE_TIME);
  }
}

void
Yeti::kill_fall()
{
  take_hit(*get_nearest_player());
}

void
Yeti::write(lisp::Writer& writer)
{
  writer.start_list("yeti");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  if(dead_script != "") {
    writer.write_string("dead-script", dead_script);
  }

  writer.end_list("yeti");
}

void
Yeti::drop_stalactite()
{
  // make a stalactite falling down and shake camera a bit
  Sector::current()->camera->shake(.1, 0, 10);

  YetiStalactite* nearest = 0;
  float dist = FLT_MAX;

  Player* player = this->get_nearest_player();
  if (!player) return;

  Sector* sector = Sector::current();
  for(Sector::GameObjects::iterator i = sector->gameobjects.begin();
      i != sector->gameobjects.end(); ++i) {
    YetiStalactite* stalactite = dynamic_cast<YetiStalactite*> (*i);
    if(stalactite && stalactite->is_hanging()) {
      float sdist 
        = fabsf(stalactite->get_pos().x - player->get_pos().x);
      if(sdist < dist) {
        nearest = stalactite;
        dist = sdist;
      }
    }
  }

  if(nearest)
    nearest->start_shaking();
  else
  {
      // TODO repopulate stalactites?
  }
}

HitResponse
Yeti::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { 
    // hit floor or roof
    physic.set_velocity_y(0);
    switch (state) {
      case JUMP_DOWN:
	run();
	break;
      case RUN:
	break;
      case JUMP_UP:
	break;
      case BE_ANGRY:
	// we just landed
	if(!stomp_timer.started()) {
	  sprite->set_action((dir==RIGHT)?"stand-right":"stand-left");
	  stomp_count++;
	  drop_stalactite();

	  // go to other side after 3 jumps
	  if(stomp_count == 3) {
	    jump_down();
	  } else {
	    // jump again
	    stomp_timer.start(STOMP_WAIT);
	  }
	}
	break;
    }
  } else 
  if(fabsf(hit.normal.x) > .5) {
    // hit wall
    jump_up();
  }

  return CONTINUE;
}

HitResponse
Yeti::collision_badguy(BadGuy& badguy, const CollisionHit& )
{
    // TODO change behaviour here?
    // Remove bouncing snowballs if we run into them.
    // I did this for now, since when the Yeti collides with the bouncing snowballs
    // he pushes them around. It might be nice to be able to just walk through them,
    // but I don't think that can be easily done.
    if (dynamic_cast<BouncingSnowball*>(&badguy))
        badguy.remove_me();

    return FORCE_MOVE;
}

HitResponse
Yeti::collision_player(Player& player, const CollisionHit& )
{
  if(player.is_invincible()) {
    kill_fall();
    return ABORT_MOVE;
  }

  // hit from above?
  if (player.get_bbox().p2.y < (bbox.p1.y + (bbox.get_height() / 4))) {
    collision_squished(player);
  }
  else {
    player.kill(Player::SHRINK);
  }

  return FORCE_MOVE;
}

IMPLEMENT_FACTORY(Yeti, "yeti")
