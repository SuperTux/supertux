//  SuperTux - Boss "Yeti"
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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

#include "badguy/yeti.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/bouncing_snowball.hpp"
#include "badguy/yeti_stalactite.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "util/reader_mapping.hpp"

#include <float.h>
#include <math.h>

namespace {
const float JUMP_DOWN_VX = 250; /**< horizontal speed while jumping off the dais */
const float JUMP_DOWN_VY = -250; /**< vertical speed while jumping off the dais */

const float RUN_VX = 350; /**< horizontal speed while running */

const float JUMP_UP_VX = 350; /**< horizontal speed while jumping on the dais */
const float JUMP_UP_VY = -700; /**< vertical speed while jumping on the dais */

const float STOMP_VY = -300; /** vertical speed while stomping on the dais */

const float RUN_DISTANCE = 1060; /** Distance between the x-coordinates of left and right end positions */
const float JUMP_SPACE = 448; /** Distance between jump position and stand position */
const float STOMP_WAIT = .5; /**< time we stay on the dais before jumping again */
const float SAFE_TIME = .5; /**< the time we are safe when tux just hit us */
const int INITIAL_HITPOINTS = 5; /**< number of hits we can take */

const float YETI_SQUISH_TIME = 5;
}

Yeti::Yeti(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/yeti/yeti.sprite"),
  state(),
  state_timer(),
  safe_timer(),
  stomp_count(),
  hit_points(),
  hud_head(),
  left_stand_x(),
  right_stand_x(),
  left_jump_x(),
  right_jump_x(),
  fixed_pos(),
  hud_icon()
{
  if ( !reader.get("lives", hit_points) ) {
    hit_points = INITIAL_HITPOINTS;
  }
  countMe = true;
  SoundManager::current()->preload("sounds/yeti_gna.wav");
  SoundManager::current()->preload("sounds/yeti_roar.wav");

  if ( !reader.get("hud-icon", hud_icon) ) {
    hud_icon = "images/creatures/yeti/hudlife.png";
  }
  hud_head = Surface::create(hud_icon);

  initialize();

  if ( !reader.get("fixed-pos", fixed_pos) ) {
    fixed_pos = false;
  }
  if (fixed_pos) {
    left_stand_x = 80;
    right_stand_x = 1140;
    left_jump_x = 528;
    right_jump_x = 692;
  } else {
    recalculate_pos();
  }
}

Yeti::~Yeti()
{
}

void
Yeti::initialize()
{
  dir = RIGHT;
  jump_down();
}

void
Yeti::recalculate_pos()
{
  if (dir == RIGHT) {
    left_stand_x = bbox.p1.x;
    right_stand_x = left_stand_x + RUN_DISTANCE;
  } else {
    right_stand_x = bbox.p1.x;
    left_stand_x = right_stand_x - RUN_DISTANCE;
  }

  left_jump_x = left_stand_x + JUMP_SPACE;
  right_jump_x = right_stand_x - JUMP_SPACE;
}

void
Yeti::draw(DrawingContext& context)
{
  // we blink when we are safe
  if(safe_timer.started() && size_t(game_time*40)%2)
    return;

  draw_hit_points(context);

  BadGuy::draw(context);
}

void
Yeti::draw_hit_points(DrawingContext& context)
{
  if (hud_head)
  {
    context.push_transform();
    context.set_translation(Vector(0, 0));

    for (int i = 0; i < hit_points; ++i)
    {
      context.draw_surface(hud_head, Vector(BORDER_X + (i * hud_head->get_width()), BORDER_Y + 1), LAYER_FOREGROUND1);
    }

    context.pop_transform();
  }
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
      if (((dir == RIGHT) && (get_pos().x >= right_jump_x)) || ((dir == LEFT) && (get_pos().x <= left_jump_x))) jump_up();
      break;
    case JUMP_UP:
      physic.set_velocity_x((dir==RIGHT)?+JUMP_UP_VX:-JUMP_UP_VX);
      if (((dir == RIGHT) && (get_pos().x >= right_stand_x)) || ((dir == LEFT) && (get_pos().x <= left_stand_x))) be_angry();
      break;
    case BE_ANGRY:
      if(state_timer.check() && on_ground()) {
        physic.set_velocity_y(STOMP_VY);
        sprite->set_action((dir==RIGHT)?"stomp-right":"stomp-left");
      }
      break;
    case SQUISHED:
      if (state_timer.check()) {
        remove_me();
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
  dir = (dir==RIGHT) ? LEFT : RIGHT;

  sprite->set_action((dir==RIGHT) ? "stand-right" : "stand-left");
  physic.set_velocity_x(0);
  stomp_count = 0;
  state = BE_ANGRY;
  state_timer.start(STOMP_WAIT);
}

bool
Yeti::collision_squished(GameObject& object)
{
  kill_squished(object);

  return true;
}

void
Yeti::kill_squished(GameObject& object)
{
  Player* player = dynamic_cast<Player*>(&object);
  if (player) {
    player->bounce(*this);
    take_hit(*player);
  }
}

void Yeti::take_hit(Player& )
{
  if(safe_timer.started())
    return;

  SoundManager::current()->play("sounds/yeti_roar.wav");
  hit_points--;

  if(hit_points <= 0) {
    // We're dead
    physic.enable_gravity(true);
    physic.set_velocity_x(0);
    physic.set_velocity_y(0);

    // Set the badguy layer to be above the foremost, so that
    // this does not reveal secret tilemaps:
    layer = Sector::current()->get_foremost_layer() + 1;
    state = SQUISHED;
    state_timer.start(YETI_SQUISH_TIME);
    set_colgroup_active(COLGROUP_MOVING_ONLY_STATIC);
    sprite->set_action("dead");

    run_dead_script();
  }
  else {
    safe_timer.start(SAFE_TIME);
  }
}

void
Yeti::kill_fall()
{
  // shooting bullets or being invincible won't work :)
}

void
Yeti::drop_stalactite()
{
  // make a stalactite falling down and shake camera a bit
  Sector::current()->camera->shake(.1f, 0, 10);

  Player* player = get_nearest_player();
  if (!player) return;

  Sector* sector = Sector::current();
  for(Sector::GameObjects::const_iterator i = sector->gameobjects.begin();
      i != sector->gameobjects.end(); ++i) {
    YetiStalactite* stalactite = dynamic_cast<YetiStalactite*>(i->get());
    if(stalactite && stalactite->is_hanging()) {
      if (hit_points >= 3) {
        // drop stalactites within 3 of player, going out with each jump
        float distancex = fabsf(stalactite->get_bbox().get_middle().x - player->get_bbox().get_middle().x);
        if(distancex < stomp_count*32) {
          stalactite->start_shaking();
        }
      }
      else { /* if (hitpoints < 3) */
        // drop every 3rd pair of stalactites
        if(((((int)stalactite->get_pos().x + 16) / 64) % 3) == (stomp_count % 3)) {
          stalactite->start_shaking();
        }
      }
    } /* if(stalactite && stalactite->is_hanging()) */
  }
}

void
Yeti::collision_solid(const CollisionHit& hit)
{
  update_on_ground_flag(hit);
  if(hit.top || hit.bottom) {
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
        if(!state_timer.started()) {
          sprite->set_action((dir==RIGHT)?"stand-right":"stand-left");
          SoundManager::current()->play("sounds/yeti_gna.wav");
          stomp_count++;
          drop_stalactite();

          // go to other side after 3 jumps
          if(stomp_count == 3) {
            jump_down();
          } else {
            // jump again
            state_timer.start(STOMP_WAIT);
          }
        }
        break;
      case SQUISHED:
        break;
    }
  } else if(hit.left || hit.right) {
    // hit wall
    jump_up();
  }
}

bool
Yeti::is_flammable() const
{
  return false;
}

ObjectSettings
Yeti::get_settings() {
  ObjectSettings result(_("Yeti"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Death script"),   &dead_script));
  result.options.push_back( ObjectOption(MN_TOGGLE,    _("Fixed position"), &fixed_pos));
  result.options.push_back( ObjectOption(MN_INTFIELD,  _("Lives"),          &hit_points));
  return result;
}

void
Yeti::save(Writer& writer) {
  BadGuy::save(writer);
  writer.write("hud-icon", hud_icon, false);
  writer.write("fixed-pos", fixed_pos);
  writer.write("lives", hit_points);
}

/* EOF */
