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

#include "badguy.hpp"
#include "object/camera.hpp"
#include "object/tilemap.hpp"
#include "tile.hpp"
#include "statistics.hpp"
#include "game_session.hpp"

static const float SQUISH_TIME = 2;
static const float X_OFFSCREEN_DISTANCE = 1600;
static const float Y_OFFSCREEN_DISTANCE = 1200;

BadGuy::BadGuy()
  : countMe(true), sprite(0), dir(LEFT), state(STATE_INIT)
{
  set_group(COLGROUP_DISABLED);
}

BadGuy::~BadGuy()
{
  delete sprite;
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
    sprite->draw(context, get_pos(), LAYER_OBJECTS);
    context.set_drawing_effect(old_effect);
  } else {
    sprite->draw(context, get_pos(), LAYER_OBJECTS);
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
	std::cout << "Warning: tried to write out a generic badguy." << std::endl;
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
      if(badguy && badguy->state == STATE_ACTIVE)
        return collision_badguy(*badguy, hit);

      Player* player = dynamic_cast<Player*> (&other);
      if(player)
        return collision_player(*player, hit);

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
  if(player.is_invincible()) {
    kill_fall();
    return ABORT_MOVE;
  }
  // hit from above?
  if(player.get_movement().y - get_movement().y > 0 && player.get_bbox().p2.y <
      (get_bbox().p1.y + get_bbox().p2.y) / 2) {
    // if it's not possible to squish us, then this will hurt
    if(!collision_squished(player))
      player.kill(Player::SHRINK);

    return FORCE_MOVE;
  }
  player.kill(Player::SHRINK);
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

void
BadGuy::kill_squished(Player& player)
{
  sound_manager->play("sounds/squish.wav", get_pos());
  physic.enable_gravity(true);
  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
  set_state(STATE_SQUISHED);
  set_group(COLGROUP_MOVING_ONLY_STATIC);
  global_stats.add_points(BADGUYS_KILLED_STAT, 1);
  player.bounce(*this);
}

void
BadGuy::kill_fall()
{
  sound_manager->play("sounds/fall.wav", get_pos());
  global_stats.add_points(BADGUYS_KILLED_STAT, 1);
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
  if (start_position.x > scroll_x - X_OFFSCREEN_DISTANCE &&
      start_position.x < scroll_x - bbox.get_width() &&
      start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
      start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) {
    dir = RIGHT;
    set_state(STATE_ACTIVE);
    activate();
  } else if (start_position.x > scroll_x &&
      start_position.x < scroll_x + X_OFFSCREEN_DISTANCE &&
      start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
      start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) {
    dir = LEFT;
    set_state(STATE_ACTIVE);
    activate();
  } else if (start_position.x > scroll_x - X_OFFSCREEN_DISTANCE &&
      start_position.x < scroll_x + X_OFFSCREEN_DISTANCE &&
      ((start_position.y > scroll_y &&
        start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) ||
       (start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
        start_position.y < scroll_y))) {
    dir = start_position.x < scroll_x ? RIGHT : LEFT;
    set_state(STATE_ACTIVE);
    activate();
  } else if(state == STATE_INIT
      && start_position.x > scroll_x - X_OFFSCREEN_DISTANCE
      && start_position.x < scroll_x + X_OFFSCREEN_DISTANCE
      && start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE
      && start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) {
    dir = LEFT;
    set_state(STATE_ACTIVE);
    activate();
  } 
}

bool
BadGuy::may_fall_off_platform()
{
  int tile_x, tile_y;
  // First, let's say the badguy moves 32 units in the
  // direction it's heading, so do some voodoo maths magic
  // to determine its future position.
  Vector pos;
  if (dir == LEFT)
    pos = Vector(bbox.p1.x - 32.f, bbox.p2.y);
  else
    pos = Vector(bbox.p2.x, bbox.p2.y);

  // Now, snap the badguy's X coordinate to the 32x32/cell grid.
  if (dir == LEFT) // use the ceiling
    tile_x = (int)ceilf(pos.x/32.0f);
  else // use the floor
    tile_x = (int)floorf(pos.x/32.0f);

  // We might be falling down, so use the ceiling to round upward and
  // get the lower position. (Positive Y goes downward.)
  tile_y = (int)ceilf(pos.y/32.0f);

#if defined(DEBUG_STAY_ON_PLATFORM)
  // Draw!
  GameSession::current()->context->draw_filled_rect(Vector(tile_x*32.0f, tile_y*32.0f), Vector(32.f, 32.f), Color(1.f, 0.f, 0.f), 999);
#endif

  // Now, if the badguy intersects with a tile, he won't fall off.
  // If he doesn't intersect, he probably will.
  // Note that the tile's Y coordinate is offset by +1 from the object's Y.
  if (Sector::current()->solids->get_tile(tile_x, tile_y)->getAttributes() & FLAG_SOLID)
  {
    // It's a solid tile. Good.
    return false;
  }

  // Watch out there buddy, you might take a sticky end!
  return true;
}
