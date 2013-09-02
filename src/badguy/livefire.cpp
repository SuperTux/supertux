//  SuperTux badguy - walking flame that glows
//  Copyright (C) 2013 LMH <lmh.0013@gmail.com>
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

#include "badguy/livefire.hpp"

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

static const float WALKSPEED = 80;
static const float MAXDROPHEIGHT = 20;

LiveFire::LiveFire(const Reader& reader) :
  WalkingBadguy(reader, "images/creatures/livefire/livefire.sprite", "left", "right"),
  lightsprite(sprite_manager->create("images/objects/lightmap_light/lightmap_light-medium.sprite")),
  state(STATE_WALKING)  
{
  walk_speed = WALKSPEED;
  max_drop_height = MAXDROPHEIGHT;
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(1.0f, 0.9f, 0.8f));
}

void
LiveFire::collision_solid(const CollisionHit& hit)
{
  if(state != STATE_WALKING) {
    BadGuy::collision_solid(hit);
    return;
  }
  WalkingBadguy::collision_solid(hit);
}

HitResponse
LiveFire::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  if(state != STATE_WALKING) {
    return BadGuy::collision_badguy(badguy, hit);
  }
  return WalkingBadguy::collision_badguy(badguy, hit);
}

void
LiveFire::active_update(float elapsed_time) {

  // Remove when extinguish animation is done
  if((sprite->get_action() == "extinguish-left" || sprite->get_action() == "extinguish-right" )
    && sprite->animation_done()) remove_me();

  if(state == STATE_WALKING) {
    WalkingBadguy::active_update(elapsed_time);
    return;
  }

  if(state == STATE_SLEEPING) {

    Player* player = this->get_nearest_player();
    if (player) {
      Rectf mb = this->get_bbox();
      Rectf pb = player->get_bbox();

      bool inReach_left = (pb.p2.x >= mb.p2.x-((dir == LEFT) ? 256 : 0));
      bool inReach_right = (pb.p1.x <= mb.p1.x+((dir == RIGHT) ? 256 : 0));
      bool inReach_top = (pb.p2.y >= mb.p1.y);
      bool inReach_bottom = (pb.p1.y <= mb.p2.y);

      if (inReach_left && inReach_right && inReach_top && inReach_bottom) {
        // wake up
        sprite->set_action(dir == LEFT ? "waking-left" : "waking-right", 1);
        state = STATE_WAKING;
      }
    }

    BadGuy::active_update(elapsed_time);
  }

  if(state == STATE_WAKING) {
    if(sprite->animation_done()) {
      // start walking
      state = STATE_WALKING;
      WalkingBadguy::initialize();
    }

    BadGuy::active_update(elapsed_time);
  }
}

void
LiveFire::draw(DrawingContext& context)
{
  //Draw the Sprite.
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
  //Draw the light
  context.push_target();
  context.set_target(DrawingContext::LIGHTMAP);
  lightsprite->draw(context, get_bbox().get_middle(), 0);
  context.pop_target();
}

void
LiveFire::freeze()
{
  // attempting to freeze a flame causes it to go out
  kill_fall();
}

bool
LiveFire::is_freezable() const
{
  return true;
}

bool
LiveFire::is_flammable() const
{
  return false;
}

void
LiveFire::kill_fall()
{
  //TODO: get unique sound for ice-fire encounters
  sound_manager->play("sounds/fall.wav", get_pos());
  // throw a puff of smoke
  Vector ppos = bbox.get_middle();
  Vector pspeed = Vector(0, -150);
  Vector paccel = Vector(0,0);
  Sector::current()->add_object(new SpriteParticle("images/objects/particles/smoke.sprite", "default", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_BACKGROUNDTILES+2));
  // extinguish the flame
  sprite->set_action(dir == LEFT ? "extinguish-left" : "extinguish-right", 1);
  physic.set_velocity_y(0);
  physic.set_acceleration_y(0);
  physic.enable_gravity(false);
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(0.5f, 0.4f, 0.3f));
  set_group(COLGROUP_DISABLED);

  // start dead-script
  run_dead_script();
}

/* The following defines a sleeping version */

LiveFireAsleep::LiveFireAsleep(const Reader& reader) :
  LiveFire(reader)
{
  state = STATE_SLEEPING;
}

void
LiveFireAsleep::initialize()
{
  physic.set_velocity_x(0);
  sprite->set_action(dir == LEFT ? "sleeping-left" : "sleeping-right");
}

/* The following defines a dormant version that never wakes */
LiveFireDormant::LiveFireDormant(const Reader& reader) :
  LiveFire(reader)
{
  walk_speed = 0;
  state = STATE_DORMANT;
}

void
LiveFireDormant::initialize()
{
  physic.set_velocity_x(0);
  sprite->set_action(dir == LEFT ? "sleeping-left" : "sleeping-right");
}

/* EOF */
