//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "badguy/kugelblitz.hpp"

#include <math.h>

#include "math/random_generator.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "util/reader.hpp"

#define  LIFETIME 5
#define  MOVETIME 0.75
#define  BASE_SPEED 200
#define  RAND_SPEED 150

Kugelblitz::Kugelblitz(const Reader& reader) :
  BadGuy(reader, "images/creatures/kugelblitz/kugelblitz.sprite"),
  pos_groundhit(),
  groundhit_pos_set(false),
  dying(),
  movement_timer(),
  lifetime(),
  direction(),
  light(0.0f,0.0f,0.0f),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light.sprite"))
{
  reader.get("x", start_position.x);
  sprite->set_action("falling");
  physic.enable_gravity(false);
  countMe = false;

  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(0.2f, 0.1f, 0.0f));
}

void
Kugelblitz::initialize()
{
  physic.set_velocity_y(300);
  physic.set_velocity_x(-20); //fall a little to the left
  direction = 1;
  dying = false;
}

void
Kugelblitz::collision_solid(const CollisionHit& chit)
{
  hit(chit);
}

HitResponse
Kugelblitz::collision_player(Player& player, const CollisionHit& )
{
  if(player.is_invincible()) {
    explode();
    return ABORT_MOVE;
  }
  // hit from above?
  if(player.get_movement().y - get_movement().y > 0 && player.get_bbox().p2.y <
     (bbox.p1.y + bbox.p2.y) / 2) {
    // if it's not is it possible to squish us, then this will hurt
    if(!collision_squished(player))
      player.kill(false);
    explode();
    return FORCE_MOVE;
  }
  player.kill(false);
  explode();
  return FORCE_MOVE;
}

HitResponse
Kugelblitz::collision_badguy(BadGuy& other , const CollisionHit& chit)
{
  //Let the Kugelblitz explode, too? The problem with that is that
  //two Kugelblitzes would cancel each other out on contact...
  other.kill_fall();
  return hit(chit);
}

HitResponse
Kugelblitz::hit(const CollisionHit& hit_)
{
  // hit floor?
  if(hit_.bottom) {
    if (!groundhit_pos_set)
    {
      pos_groundhit = get_pos();
      groundhit_pos_set = true;
    }
    sprite->set_action("flying");
    physic.set_velocity_y(0);
    //Set random initial speed and direction
    direction = gameRandom.rand(2)? 1: -1;
    int speed = (BASE_SPEED + (gameRandom.rand(RAND_SPEED))) * direction;
    physic.set_velocity_x(speed);
    movement_timer.start(MOVETIME);
    lifetime.start(LIFETIME);

  } else if(hit_.top) { // bumped on roof
    physic.set_velocity_y(0);
  }

  return CONTINUE;
}

void
Kugelblitz::active_update(float elapsed_time)
{
  if (lifetime.check()) {
    explode();
  }
  else {
    if (groundhit_pos_set) {
      if (movement_timer.check()) {
        if (direction == 1) direction = -1; else direction = 1;
        int speed = (BASE_SPEED + (gameRandom.rand(RAND_SPEED))) * direction;
        physic.set_velocity_x(speed);
        movement_timer.start(MOVETIME);
      }
    }
    /*
      if (Sector::current()->solids->get_tile_at(get_pos())->getAttributes() == 16) {
      //HIT WATER
      Sector::current()->add_object(new Electrifier(75,1421,1.5));
      Sector::current()->add_object(new Electrifier(76,1422,1.5));
      explode();
      }
      if (Sector::current()->solids->get_tile_at(get_pos())->getAttributes() == 48) {
      //HIT ELECTRIFIED WATER
      explode();
      }
    */
  }
  BadGuy::active_update(elapsed_time);
}

void
Kugelblitz::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), layer);

  //Only draw light in dark areas
  context.get_light( bbox.get_middle(), &light );
  if (light.red + light.green < 2.0){
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    sprite->draw(context, get_pos(), layer);
    lightsprite->draw(context, bbox.get_middle(), 0);
    context.pop_target();
  }
}

void
Kugelblitz::kill_fall()
{
}

void
Kugelblitz::explode()
{
  if (!dying) {
    sprite->set_action("pop");
    lifetime.start(0.2f);
    dying = true;
  }
  else remove_me();
}

void
Kugelblitz::try_activate()
{
  // Much smaller offscreen distances to pop out of nowhere and surprise Tux
  float X_OFFSCREEN_DISTANCE = 400;
  float Y_OFFSCREEN_DISTANCE = 600;

  Player* player_ = get_nearest_player();
  if (!player_) return;
  Vector dist = player_->get_bbox().get_middle() - bbox.get_middle();
  if ((fabsf(dist.x) <= X_OFFSCREEN_DISTANCE) && (fabsf(dist.y) <= Y_OFFSCREEN_DISTANCE)) {
    set_state(STATE_ACTIVE);
    if (!is_initialized) {

      // if starting direction was set to AUTO, this is our chance to re-orient the badguy
      if (start_dir == AUTO) {
        Player* player__ = get_nearest_player();
        if (player__ && (player__->get_bbox().p1.x > bbox.p2.x)) {
          dir = RIGHT;
        } else {
          dir = LEFT;
        }
      }

      initialize();
      is_initialized = true;
    }
    activate();
  }
}

ObjectSettings
Kugelblitz::get_settings() {
  ObjectSettings result(_("Kugelblitz"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Death script"), &dead_script));
  return result;
}

bool
Kugelblitz::is_flammable() const
{
  return false;
}

/* EOF */
