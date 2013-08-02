//  IceCrusher - A block to stand on, which can drop down to crush the player
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "object/icecrusher.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "object/camera.hpp"
#include "object/particles.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

namespace {
/* Maximum movement speed in pixels per LOGICAL_FPS */
const float MAX_DROP_SPEED = 10.0;
const float RECOVER_SPEED_NORMAL = -3.125;
const float RECOVER_SPEED_LARGE  = -2.0;
const float DROP_ACTIVATION_DISTANCE = 4.0;
const float PAUSE_TIME_NORMAL = 0.5;
const float PAUSE_TIME_LARGE  = 1.0;
}

IceCrusher::IceCrusher(const Reader& reader) :
  MovingSprite(reader, "images/creatures/icecrusher/icecrusher.sprite", LAYER_OBJECTS, COLGROUP_STATIC),
  state(IDLE), 
  start_position(),
  physic(),
  cooldown_timer(0.0),
  lefteye(),
  righteye(),
  whites(),
  ic_size(NORMAL)
{
  // TODO: icecrusher hitting deserves its own sounds-
  // one for hitting the ground, one for hitting Tux
  sound_manager->preload("sounds/brick.wav");

  start_position = get_bbox().p1;
  set_state(state, true);
  
  float sprite_width = sprite->get_width ();
  if (sprite_width >= 128.0)
    ic_size = LARGE;

  lefteye = sprite_manager->create(sprite_name);
  lefteye->set_action("lefteye");
  righteye = sprite_manager->create(sprite_name);
  righteye->set_action("righteye");
  whites = sprite_manager->create(sprite_name);
  whites->set_action("whites");
}

/*
  IceCrusher::IceCrusher(const IceCrusher& other)
  : MovingSprite(other), 
  state(other.state), speed(other.speed) 
  {
  start_position = get_bbox().p1;
  set_state(state, true);
  }
*/
void 
IceCrusher::set_state(IceCrusherState state, bool force) 
{
  if ((this->state == state) && (!force)) return;
  switch(state) {
    case IDLE:
      set_group(COLGROUP_STATIC);
      physic.enable_gravity (false);
      sprite->set_action("idle");
      break;
    case CRUSHING:
      set_group(COLGROUP_MOVING_STATIC);
      physic.reset ();
      physic.enable_gravity (true);
      sprite->set_action("crushing");
      break;
    case RECOVERING:
      set_group(COLGROUP_MOVING_STATIC);
      physic.enable_gravity (false);
      sprite->set_action("recovering");
      break;
    default:
      log_debug << "IceCrusher in invalid state" << std::endl;
      break;
  }
  this->state = state;
}

HitResponse
IceCrusher::collision(GameObject& other, const CollisionHit& hit)
{
  Player* player = dynamic_cast<Player*>(&other);

  /* If the other object is the player, and the collision is at the bottom of
   * the ice crusher, hurt the player. */
  if (player && hit.bottom) {
    sound_manager->play("sounds/brick.wav");
    if(player->is_invincible()) {
      if (state == CRUSHING)
        set_state(RECOVERING);
      return ABORT_MOVE;
    }
    player->kill(false);
    if (state == CRUSHING)
      set_state(RECOVERING);
    return FORCE_MOVE;
  }
  BadGuy* badguy = dynamic_cast<BadGuy*>(&other);
  if (badguy) {
    badguy->kill_fall();
  }
  return FORCE_MOVE;
}
    
void 
IceCrusher::collision_solid(const CollisionHit& hit)
{
  switch(state) {
    case IDLE:
      break;
    case CRUSHING:
      if (hit.bottom) {
        if (ic_size == LARGE) {
          cooldown_timer = PAUSE_TIME_LARGE;
          Sector::current()->camera->shake (/* frequency = */ .125f, /* x = */ 0.0, /* y = */ 16.0);
          sound_manager->play("sounds/brick.wav");
          // throw some particles, bigger and more for large icecrusher
          for(int j = 0; j < 9; j++)
          {
          Sector::current()->add_object(
            new Particles(Vector(get_bbox().p2.x - j*8 - 4, get_bbox().p2.y),
              0, 90-5*j, Vector(140, -380), Vector(0, 300),
              1, Color(.6f, .6f, .6f), 5, 1.8f, LAYER_OBJECTS+1));
          Sector::current()->add_object(
            new Particles(Vector(get_bbox().p1.x + j*8 + 4, get_bbox().p2.y),
              270+5*j, 360, Vector(140, -380), Vector(0, 300),
              1, Color(.6f, .6f, .6f), 5, 1.8f, LAYER_OBJECTS+1));
          }
        }
        else {
          cooldown_timer = PAUSE_TIME_NORMAL;
          Sector::current()->camera->shake (/* frequency = */ .1f, /* x = */ 0.0, /* y = */ 8.0);
          sound_manager->play("sounds/brick.wav");
          // throw some particles
          for(int j = 0; j < 5; j++)
          {
          Sector::current()->add_object(
            new Particles(Vector(get_bbox().p2.x - j*8 - 4, get_bbox().p2.y),
              0, 90+10*j, Vector(140, -260), Vector(0, 300),
              1, Color(.6f, .6f, .6f), 4, 1.6f, LAYER_OBJECTS+1));
          Sector::current()->add_object(
            new Particles(Vector(get_bbox().p1.x + j*8 + 4, get_bbox().p2.y),
              270+10*j, 360, Vector(140, -260), Vector(0, 300),
              1, Color(.6f, .6f, .6f), 4, 1.6f, LAYER_OBJECTS+1));
          }
        }
        set_state(RECOVERING);
      }
      break;
    case RECOVERING:
      break;
    default:
      log_debug << "IceCrusher in invalid state" << std::endl;
      break;
  }
}

void
IceCrusher::update(float elapsed_time)
{
  if (cooldown_timer >= elapsed_time)
  {
    cooldown_timer -= elapsed_time;
    return;
  }
  else if (cooldown_timer != 0.0)
  {
    elapsed_time -= cooldown_timer;
    cooldown_timer = 0.0;
  }

  switch(state) {
    case IDLE:
      movement = Vector (0, 0);
      if (found_victim())
        set_state(CRUSHING);
      break;
    case CRUSHING:
      movement = physic.get_movement (elapsed_time);
      if (movement.y > MAX_DROP_SPEED)
        movement.y = MAX_DROP_SPEED;
      break;
    case RECOVERING:
      if (get_bbox().p1.y <= start_position.y+1) {
        set_pos(start_position);
        movement = Vector (0, 0);
        if (ic_size == LARGE)
          cooldown_timer = PAUSE_TIME_LARGE;
        else
          cooldown_timer = PAUSE_TIME_NORMAL;
        set_state(IDLE);
      }
      else {
        if (ic_size == LARGE)
          movement = Vector (0, RECOVER_SPEED_LARGE);
        else
          movement = Vector (0, RECOVER_SPEED_NORMAL);
      }
      break;
    default:
      log_debug << "IceCrusher in invalid state" << std::endl;
      break;
  }
}

void
IceCrusher::draw(DrawingContext& context)
{
  context.push_target();
  context.set_target(DrawingContext::NORMAL);
  sprite->draw(context, get_pos(), layer);
  if(!(state == CRUSHING) && sprite->has_action("whites"))
  {
    // draw icecrusher's eyes slightly behind
    lefteye->draw(context, get_pos()+eye_position(false), layer-1);
    righteye->draw(context, get_pos()+eye_position(true), layer-1);
    // draw the whites of icecrusher's eyes even further behind
    whites->draw(context, get_pos(), layer-2);
  }
  context.pop_target();
}

bool
IceCrusher::found_victim()
{
  Player* player = Sector::current()->get_nearest_player (this->get_bbox ());
  if (!player) return false;

  const Rectf& player_bbox = player->get_bbox();
  const Rectf& crusher_bbox = get_bbox();
  if ((player_bbox.p1.y >= crusher_bbox.p2.y) /* player is below crusher */
      && (player_bbox.p2.x > (crusher_bbox.p1.x - DROP_ACTIVATION_DISTANCE))
      && (player_bbox.p1.x < (crusher_bbox.p2.x + DROP_ACTIVATION_DISTANCE)))
    return true;
  else
    return false;
}

Vector
IceCrusher::eye_position(bool right)
{
  if(state == IDLE)
  {
    Player* player = Sector::current()->get_nearest_player (this->get_bbox ());
    if(player)
    {
      // Icecrusher focuses on approximate position of player's head
      const float player_focus_x = (player->get_bbox().p2.x + player->get_bbox().p1.x) * 0.5;
      const float player_focus_y = player->get_bbox().p2.y * 0.25 + player->get_bbox().p1.y * 0.75;
      // Icecrusher's approximate origin of line-of-sight
      const float crusher_origin_x = (get_bbox().p2.x + get_bbox().p1.x) * 0.5;
      const float crusher_origin_y = (get_bbox().p2.y + get_bbox().p1.y) * 0.5;
      // Line-of-sight displacement from icecrusher to player
      const float displacement_x = player_focus_x - crusher_origin_x;
      const float displacement_y = player_focus_y - crusher_origin_y;
      const float displacement_mag = pow(pow(displacement_x, 2.0) + pow(displacement_y, 2.0), 0.5);
      // Determine weighting for eye displacement along x given icecrusher eye shape
      int weight_x = sprite->get_width()/64 * ((displacement_x > 0) == right) ? 1 : 4;
      int weight_y = sprite->get_width()/64 * 2;

      return Vector(displacement_x/displacement_mag * weight_x, displacement_y/displacement_mag * weight_y - weight_y);
    }
  }
  else if(state == RECOVERING)
  {
    // Eyes spin while icecrusher is recovering, giving a dazed impression
    return Vector(sin((right ? 1 : -1) * get_pos().y/13) * sprite->get_width()/64 * 2 - (right ? 1 : -1) * sprite->get_width()/64 * 2,
                  cos(((right ? 3.1415 : 0) + get_pos().y/13)) * sprite->get_width()/64 * 2 - sprite->get_width()/64 * 2);
  }

  return Vector(0,0);
}

/* EOF */
