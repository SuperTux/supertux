//  SuperTux - "Totem" Badguy
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "badguy/totem.hpp"

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"

#include <math.h>

static const float JUMP_ON_SPEED_Y = -400;
static const float JUMP_OFF_SPEED_Y = -500;
static const std::string LAND_ON_TOTEM_SOUND = "sounds/totem.ogg";

Totem::Totem(const Reader& reader) :
  BadGuy(reader, "images/creatures/totem/totem.sprite"),
  carrying(0),
  carried_by(0)
{
  SoundManager::current()->preload( LAND_ON_TOTEM_SOUND );
}

Totem::~Totem()
{
  if (carrying) carrying->jump_off();
  if (carried_by) jump_off();
}

bool
Totem::updatePointers(const GameObject* from_object, GameObject* to_object)
{
  if (from_object == carrying) {
    carrying = dynamic_cast<Totem*>(to_object);
    return true;
  }
  if (from_object == carried_by) {
    carried_by = dynamic_cast<Totem*>(to_object);
    return true;
  }
  return false;
}

void
Totem::initialize()
{
  if (!carried_by) {
static const float WALKSPEED = 100;
    physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
    sprite->set_action(dir == LEFT ? "walking-left" : "walking-right");
    return;
  } else {
    synchronize_with(carried_by);
    sprite->set_action(dir == LEFT ? "stacked-left" : "stacked-right");
    return;
  }
}

void
Totem::active_update(float elapsed_time)
{
  BadGuy::active_update(elapsed_time);

  if (!carried_by) {
    if (on_ground() && might_fall())
    {
      dir = (dir == LEFT ? RIGHT : LEFT);
      initialize();
    }

    Sector* s = Sector::current();
    if (s) {
      // jump a bit if we find a suitable totem
      for (std::vector<MovingObject*>::iterator i = s->moving_objects.begin(); i != s->moving_objects.end(); ++i) {
        Totem* t = dynamic_cast<Totem*>(*i);
        if (!t) continue;

        // skip if we are not approaching each other
        if (!((dir == LEFT) && (t->dir == RIGHT))) continue;

        Vector p1 = bbox.p1;
        Vector p2 = t->get_pos();

        // skip if not on same height
        float dy = (p1.y - p2.y);
        if (fabsf(dy - 0) > 2) continue;

        // skip if too far away
        float dx = (p1.x - p2.x);
        if (fabsf(dx - 128) > 2) continue;

        physic.set_velocity_y(JUMP_ON_SPEED_Y);
        p1.y -= 1;
        this->set_pos(p1);
        break;
      }
    }
  }

  if (carried_by) {
    this->synchronize_with(carried_by);
  }

  if (carrying) {
    carrying->synchronize_with(this);
  }

}

bool
Totem::collision_squished(GameObject& object)
{
  /// Tux shouldn't be able to bisect totem stack by sacrificing his powerup.
  /// --Hume2
  if (carrying) {
    return false;
  }

  if (carried_by) {
    Player* player = dynamic_cast<Player*>(&object);
    if (player) player->bounce(*this);
    jump_off();
  }

  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  kill_squished(object);
  return true;
}

void
Totem::collision_solid(const CollisionHit& hit)
{
  update_on_ground_flag(hit);

  // if we are being carried around, pass event to bottom of stack and ignore it
  if (carried_by) {
    carried_by->collision_solid(hit);
    return;
  }

  // If we hit something from above or below: stop moving in this direction
  if (hit.top || hit.bottom) {
    physic.set_velocity_y(0);
  }

  // If we are hit from the direction we are facing: turn around
  if (hit.left && (dir == LEFT)) {
    dir = RIGHT;
    initialize();
  }
  if (hit.right && (dir == RIGHT)) {
    dir = LEFT;
    initialize();
  }
}

HitResponse
Totem::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  // if we are being carried around, pass event to bottom of stack and ignore it
  if (carried_by) {
    carried_by->collision_badguy(badguy, hit);
    return CONTINUE;
  }

  // if we hit a Totem that is not from our stack: have our base jump on its top
  Totem* totem = dynamic_cast<Totem*>(&badguy);
  if (totem) {
    Totem* thisBase = this; while (thisBase->carried_by) thisBase=thisBase->carried_by;
    Totem* srcBase = totem; while (srcBase->carried_by)  srcBase=srcBase->carried_by;
    Totem* thisTop = this;  while (thisTop->carrying)    thisTop=thisTop->carrying;
    if (srcBase != thisBase) {
      srcBase->jump_on(thisTop);
    }
  }

  // If we are hit from the direction we are facing: turn around
  if(hit.left && (dir == LEFT)) {
    dir = RIGHT;
    initialize();
  }
  if(hit.right && (dir == RIGHT)) {
    dir = LEFT;
    initialize();
  }

  return CONTINUE;
}

void
Totem::kill_fall()
{
  if (carrying) carrying->jump_off();
  if (carried_by) jump_off();

  BadGuy::kill_fall();
}

void
Totem::jump_on(Totem* target)
{
  if (target->carrying) {
    log_warning << "target is already carrying someone" << std::endl;
    return;
  }

  target->carrying = this;

  this->carried_by = target;
  this->initialize();
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  SoundManager::current()->play( LAND_ON_TOTEM_SOUND , get_pos());

  this->synchronize_with(target);
}

void
Totem::jump_off() {
  if (!carried_by) {
    log_warning << "not carried by anyone" << std::endl;
    return;
  }

  carried_by->carrying = 0;

  this->carried_by = 0;

  this->initialize();
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());

  physic.set_velocity_y(JUMP_OFF_SPEED_Y);
}

void
Totem::synchronize_with(Totem* base)
{

  if (dir != base->dir) {
    dir = base->dir;
    sprite->set_action(dir == LEFT ? "stacked-left" : "stacked-right");
  }

  Vector pos = base->get_pos();
  pos.y -= sprite->get_current_hitbox_height();
  set_pos(pos);

  physic.set_velocity_x(base->physic.get_velocity_x());
  physic.set_velocity_y(base->physic.get_velocity_y());
}


ObjectSettings
Totem::get_settings() {
  ObjectSettings result(_("Totem"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Death script"), &dead_script));
  return result;
}

/* EOF */
