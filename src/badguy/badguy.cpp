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

#include "badguy.hpp"
#include "object/camera.hpp"
#include "object/tilemap.hpp"
#include "tile.hpp"
#include "statistics.hpp"
#include "game_session.hpp"
#include "log.hpp"
#include "level.hpp"
#include "object/bullet.hpp"
#include "main.hpp"

static const float SQUISH_TIME = 2;
static const float X_OFFSCREEN_DISTANCE = 1600;
static const float Y_OFFSCREEN_DISTANCE = 1200;

BadGuy::BadGuy(const Vector& pos, const std::string& sprite_name, int layer)
  : MovingSprite(pos, sprite_name, layer, COLGROUP_DISABLED), countMe(true), dir(LEFT), start_dir(AUTO), state(STATE_INIT) 
{
  start_position = bbox.p1;

  sound_manager->preload("sounds/squish.wav");
  sound_manager->preload("sounds/fall.wav");
}

BadGuy::BadGuy(const Vector& pos, Direction direction, const std::string& sprite_name, int layer)
  : MovingSprite(pos, sprite_name, layer, COLGROUP_DISABLED), countMe(true), dir(direction), start_dir(direction), state(STATE_INIT) 
{
  start_position = bbox.p1;

  sound_manager->preload("sounds/squish.wav");
  sound_manager->preload("sounds/fall.wav");
}

BadGuy::BadGuy(const lisp::Lisp& reader, const std::string& sprite_name, int layer)
  : MovingSprite(reader, sprite_name, layer, COLGROUP_DISABLED), countMe(true), dir(LEFT), start_dir(AUTO), state(STATE_INIT) 
{
  start_position = bbox.p1;

  std::string dir_str = "auto";
  reader.get("direction", dir_str);
  start_dir = str2dir( dir_str );
  dir = start_dir;

  sound_manager->preload("sounds/squish.wav");
  sound_manager->preload("sounds/fall.wav");
}

void
BadGuy::draw(DrawingContext& context)
{
  if(!sprite)
    return;
  if(state == STATE_INIT || state == STATE_INACTIVE)
    return;
  if(state == STATE_FALLING) {
    DrawingEffect old_effect = context.get_drawing_effect();
    context.set_drawing_effect((DrawingEffect) (old_effect | VERTICAL_FLIP));
    sprite->draw(context, get_pos(), layer);
    context.set_drawing_effect(old_effect);
  } else {
    sprite->draw(context, get_pos(), layer);
  }
}

void
BadGuy::update(float elapsed_time)
{
  if(!Sector::current()->inside(bbox)) {
    remove_me();
    return;
  }
  if(is_offscreen()) {
    if (state == STATE_ACTIVE) deactivate();
    set_state(STATE_INACTIVE);
  }
  
  switch(state) {
    case STATE_ACTIVE:
      active_update(elapsed_time);
      break;
    case STATE_INIT:
    case STATE_INACTIVE:
      inactive_update(elapsed_time);
      try_activate();
      break;
    case STATE_SQUISHED:
      if(state_timer.check()) {
        remove_me();
        break;
      }
      movement = physic.get_movement(elapsed_time);
      break;
    case STATE_FALLING:
      movement = physic.get_movement(elapsed_time);
      break;
  }
}

Direction
BadGuy::str2dir( std::string dir_str )
{
  if( dir_str == "auto" )
    return AUTO;
  if( dir_str == "left" )
    return LEFT;
  if( dir_str == "right" ) 
    return RIGHT;

  //default to "auto"
  log_warning << "Badguy::str2dir: unknown direction \"" << dir_str << "\"\n";    
  return dir;
}

void
BadGuy::activate()
{
}

void
BadGuy::deactivate()
{
}

void
BadGuy::save(lisp::Writer& )
{
	log_warning << "tried to write out a generic badguy" << std::endl;
}

void
BadGuy::active_update(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time);
}

void
BadGuy::inactive_update(float )
{
}

void
BadGuy::collision_tile(uint32_t tile_attributes)
{
  if(tile_attributes & Tile::HURTS)
    kill_fall();
}

HitResponse
BadGuy::collision(GameObject& other, const CollisionHit& hit)
{
  switch(state) {
    case STATE_INIT:
    case STATE_INACTIVE:
      return ABORT_MOVE;
    case STATE_ACTIVE: {
      if(other.get_flags() & FLAG_SOLID)
        return collision_solid(other, hit);

      BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
      if(badguy && badguy->state == STATE_ACTIVE && badguy->get_group() == COLGROUP_MOVING)
        return collision_badguy(*badguy, hit);

      Player* player = dynamic_cast<Player*> (&other);
      if(player)
        return collision_player(*player, hit);

      Bullet* bullet = dynamic_cast<Bullet*> (&other);
      if(bullet)
        return collision_bullet(*bullet, hit);

      return FORCE_MOVE;
    }
    case STATE_SQUISHED:
      if(other.get_flags() & FLAG_SOLID)
        return CONTINUE;
      return FORCE_MOVE;
    case STATE_FALLING:
      return FORCE_MOVE;
  }

  return ABORT_MOVE;
}

HitResponse
BadGuy::collision_solid(GameObject& , const CollisionHit& )
{
  return FORCE_MOVE;
}

HitResponse
BadGuy::collision_player(Player& player, const CollisionHit& )
{
  /*
  printf("PlayerHit: GT %3.1f PM: %3.1f %3.1f BM: %3.1f %3.1f Hit: %3.1f %3.1f\n",
          game_time,
          player.get_movement().x, player.get_movement().y,
          get_movement().x, get_movement().y,
          hit.normal.x, hit.normal.y);
  */

  // hit from above?
  if(player.get_bbox().p2.y < (bbox.p1.y + 16)) {
    // if it's not possible to squish us, then this will hurt
    if(collision_squished(player))
      return ABORT_MOVE;
  }

  if(player.is_invincible()) {
    kill_fall();
    return ABORT_MOVE;
  }

  player.kill(false);
  return FORCE_MOVE;
}

HitResponse
BadGuy::collision_badguy(BadGuy& , const CollisionHit& )
{
  return FORCE_MOVE;
}

bool
BadGuy::collision_squished(Player& )
{
  return false;
}

HitResponse
BadGuy::collision_bullet(Bullet& , const CollisionHit& )
{
  kill_fall();
  return ABORT_MOVE;
}

void
BadGuy::kill_squished(Player& player)
{
  sound_manager->play("sounds/squish.wav", get_pos());
  physic.enable_gravity(true);
  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
  set_state(STATE_SQUISHED);
  set_group(COLGROUP_MOVING_ONLY_STATIC);
  if (countMe) Sector::current()->get_level()->stats.badguys++;
  player.bounce(*this);
}

void
BadGuy::kill_fall()
{
  sound_manager->play("sounds/fall.wav", get_pos());
  if (countMe) Sector::current()->get_level()->stats.badguys++;
  physic.set_velocity_y(0);
  physic.enable_gravity(true);
  set_state(STATE_FALLING);
}

void
BadGuy::set_state(State state)
{
  if(this->state == state)
    return;

  State laststate = this->state;
  this->state = state;
  switch(state) {
    case STATE_SQUISHED:
      state_timer.start(SQUISH_TIME);
      break;
    case STATE_ACTIVE:
      set_group(COLGROUP_MOVING);
      bbox.set_pos(start_position);
      break;
    case STATE_INACTIVE:
      // was the badguy dead anyway?
      if(laststate == STATE_SQUISHED || laststate == STATE_FALLING) {
        remove_me();
      }
      set_group(COLGROUP_DISABLED);
      break;
    case STATE_FALLING:
      set_group(COLGROUP_DISABLED);
      break;
    default:
      break;
  }
}

bool
BadGuy::is_offscreen()
{
  float scroll_x = Sector::current()->camera->get_translation().x;
  float scroll_y = Sector::current()->camera->get_translation().y;
     
  if(bbox.p2.x < scroll_x - X_OFFSCREEN_DISTANCE
      || bbox.p1.x > scroll_x + X_OFFSCREEN_DISTANCE
      || bbox.p2.y < scroll_y - Y_OFFSCREEN_DISTANCE
      || bbox.p1.y > scroll_y + Y_OFFSCREEN_DISTANCE)
    return true;

  return false;
}

void
BadGuy::try_activate()
{
  float scroll_x = Sector::current()->camera->get_translation().x;
  float scroll_y = Sector::current()->camera->get_translation().y;

  /* Activate badguys if they're just around the screen to avoid
   * the effect of having badguys suddenly popping up from nowhere.
   */
  //Badguy left of screen
  if (start_position.x > scroll_x - X_OFFSCREEN_DISTANCE && 
      start_position.x < scroll_x - bbox.get_width() &&
      start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
      start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) {
    if (start_dir != AUTO) dir = start_dir; else dir = RIGHT;
    set_state(STATE_ACTIVE);
    activate();
  //Badguy right of screen
  } else if (start_position.x > scroll_x +  SCREEN_WIDTH &&
      start_position.x < scroll_x + SCREEN_WIDTH + X_OFFSCREEN_DISTANCE &&
      start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
      start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) {
    if (start_dir != AUTO) dir = start_dir; else dir = LEFT;
    set_state(STATE_ACTIVE);
    activate();
  //Badguy over or under screen
  } else if (start_position.x > scroll_x - X_OFFSCREEN_DISTANCE &&
       start_position.x < scroll_x + X_OFFSCREEN_DISTANCE &&
       ((start_position.y > scroll_y + SCREEN_HEIGHT &&
         start_position.y < scroll_y + SCREEN_HEIGHT + Y_OFFSCREEN_DISTANCE) ||
        (start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
         start_position.y < scroll_y - bbox.get_height()  ))) {
     if (start_dir != AUTO) dir = start_dir; else dir = start_position.x < scroll_x ? RIGHT : LEFT;
     set_state(STATE_ACTIVE);
     activate();
  } else if(state == STATE_INIT
      && start_position.x > scroll_x - X_OFFSCREEN_DISTANCE
      && start_position.x < scroll_x + X_OFFSCREEN_DISTANCE
      && start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE
      && start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) {
    if (start_dir != AUTO) {
      dir = start_dir;
    } else {
      // if nearest player is to our right, start facing right
      Player* player = get_nearest_player();
      if (player && (player->get_bbox().p1.x > get_bbox().p2.x)) {
	dir = RIGHT;
      } else {
	dir = LEFT;
      }
    }
    set_state(STATE_ACTIVE);
    activate();
  } 
}

bool
BadGuy::might_fall(int height)
{
  // make sure we check for at least a 1-pixel fall
  assert(height > 0);

  float x1;
  float x2;
  float y1 = bbox.p2.y + 1;
  float y2 = bbox.p2.y + 1 + height;
  if (dir == LEFT) {
    x1 = bbox.p1.x - 1;
    x2 = bbox.p1.x - 1;
  } else {
    x1 = bbox.p2.x + 1;
    x2 = bbox.p2.x + 1;
  }
  return Sector::current()->is_free_space(Rect(x1, y1, x2, y2));
}

Player* 
BadGuy::get_nearest_player()
{
  // FIXME: does not really return nearest player

  std::vector<Player*> players = Sector::current()->get_players();
  for (std::vector<Player*>::iterator playerIter = players.begin(); playerIter != players.end(); ++playerIter) {
    Player* player = *playerIter;
    return player;
  }

  return 0;
}
