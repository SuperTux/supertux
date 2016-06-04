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

#include "badguy/badguy.hpp"

#include "audio/sound_manager.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "math/random_generator.hpp"
#include "object/broken_brick.hpp"
#include "editor/editor.hpp"
#include "object/bullet.hpp"
#include "object/particles.hpp"
#include "object/sprite_particle.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "object/water_drop.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "util/reader_mapping.hpp"

#include <math.h>
#include <sstream>

static const float SQUISH_TIME = 2;
static const float GEAR_TIME = 2;
static const float BURN_TIME = 1;

static const float X_OFFSCREEN_DISTANCE = 1280;
static const float Y_OFFSCREEN_DISTANCE = 800;

BadGuy::BadGuy(const Vector& pos, const std::string& sprite_name_, int layer_,
               const std::string& light_sprite_name) :
  MovingSprite(pos, sprite_name_, layer_, COLGROUP_DISABLED),
  physic(),
  countMe(true),
  is_initialized(false),
  start_position(),
  dir(LEFT),
  start_dir(AUTO),
  frozen(false),
  ignited(false),
  in_water(false),
  dead_script(),
  melting_time(0),
  lightsprite(SpriteManager::current()->create(light_sprite_name)),
  glowing(false),
  state(STATE_INIT),
  is_active_flag(),
  state_timer(),
  on_ground_flag(false),
  floor_normal(),
  colgroup_active(COLGROUP_MOVING)
{
  start_position = bbox.p1;

  SoundManager::current()->preload("sounds/squish.wav");
  SoundManager::current()->preload("sounds/fall.wav");
  SoundManager::current()->preload("sounds/splash.ogg");
  SoundManager::current()->preload("sounds/fire.ogg");

  dir = (start_dir == AUTO) ? LEFT : start_dir;
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
}

BadGuy::BadGuy(const Vector& pos, Direction direction, const std::string& sprite_name_, int layer_,
               const std::string& light_sprite_name) :
  MovingSprite(pos, sprite_name_, layer_, COLGROUP_DISABLED),
  physic(),
  countMe(true),
  is_initialized(false),
  start_position(),
  dir(direction),
  start_dir(direction),
  frozen(false),
  ignited(false),
  in_water(false),
  dead_script(),
  melting_time(0),
  lightsprite(SpriteManager::current()->create(light_sprite_name)),
  glowing(false),
  state(STATE_INIT),
  is_active_flag(),
  state_timer(),
  on_ground_flag(false),
  floor_normal(),
  colgroup_active(COLGROUP_MOVING)
{
  start_position = bbox.p1;

  SoundManager::current()->preload("sounds/squish.wav");
  SoundManager::current()->preload("sounds/fall.wav");
  SoundManager::current()->preload("sounds/splash.ogg");
  SoundManager::current()->preload("sounds/fire.ogg");

  dir = (start_dir == AUTO) ? LEFT : start_dir;
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
}

BadGuy::BadGuy(const ReaderMapping& reader, const std::string& sprite_name_, int layer_,
               const std::string& light_sprite_name) :
  MovingSprite(reader, sprite_name_, layer_, COLGROUP_DISABLED),
  physic(),
  countMe(true),
  is_initialized(false),
  start_position(),
  dir(LEFT),
  start_dir(AUTO),
  frozen(false),
  ignited(false),
  in_water(false),
  dead_script(),
  melting_time(0),
  lightsprite(SpriteManager::current()->create(light_sprite_name)),
  glowing(false),
  state(STATE_INIT),
  is_active_flag(),
  state_timer(),
  on_ground_flag(false),
  floor_normal(),
  colgroup_active(COLGROUP_MOVING)
{
  start_position = bbox.p1;

  std::string dir_str = "auto";
  reader.get("direction", dir_str);
  start_dir = str2dir( dir_str );
  dir = start_dir;

  reader.get("dead-script", dead_script);

  SoundManager::current()->preload("sounds/squish.wav");
  SoundManager::current()->preload("sounds/fall.wav");
  SoundManager::current()->preload("sounds/splash.ogg");
  SoundManager::current()->preload("sounds/fire.ogg");

  dir = (start_dir == AUTO) ? LEFT : start_dir;
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
}

void
BadGuy::draw(DrawingContext& context)
{
  if(!sprite.get())
    return;
  if(state == STATE_INIT || state == STATE_INACTIVE)
    return;
  if(state == STATE_FALLING) {
    context.push_transform();
    context.set_drawing_effect(context.get_drawing_effect() ^ VERTICAL_FLIP);
    sprite->draw(context, get_pos(), layer);
    context.pop_transform();
  } else {
    sprite->draw(context, get_pos(), layer);
  }

  if (glowing) {
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    lightsprite->draw(context, bbox.get_middle(), 0);
    context.pop_target();
  }
}

void
BadGuy::update(float elapsed_time)
{
  if(!Sector::current()->inside(bbox)) {
    run_dead_script();
    is_active_flag = false;
    remove_me();
    // This was removed due to fixing a bug. If is it needed somewhere, then I'm sorry. --Hume2
    /**if(countMe) {
      // get badguy name from sprite_name ignoring path and extension
      std::string badguy = sprite_name.substr(0, sprite_name.length() - 7);
      int path_chars = badguy.rfind("/",badguy.length());
      badguy = badguy.substr(path_chars + 1, badguy.length() - path_chars);
      // log warning since badguys_killed can no longer reach total_badguys
      std::string current_level = "[" + Sector::current()->get_level()->filename + "] ";
      log_warning << current_level << "Counted badguy " << badguy << " starting at " << start_position << " has left the sector" <<std::endl;;
    }*/
    return;
  }
  if ((state != STATE_INACTIVE) && is_offscreen()) {
    if (state == STATE_ACTIVE) deactivate();
    set_state(STATE_INACTIVE);
  }

  switch(state) {
    case STATE_ACTIVE:
      is_active_flag = true;
      if (Editor::is_active()) {
        break;
      }
      active_update(elapsed_time);
      break;
    case STATE_INIT:
    case STATE_INACTIVE:
      is_active_flag = false;
      inactive_update(elapsed_time);
      try_activate();
      break;
    case STATE_BURNING: {
      is_active_flag = false;
      movement = physic.get_movement(elapsed_time);
      if ( sprite->animation_done() ) {
        remove_me();
      }
    } break;
    case STATE_GEAR:
    case STATE_SQUISHED:
      is_active_flag = false;
      if(state_timer.check()) {
        remove_me();
        break;
      }
      movement = physic.get_movement(elapsed_time);
      break;
    case STATE_MELTING: {
      is_active_flag = false;
      movement = physic.get_movement(elapsed_time);
      if ( sprite->animation_done() || on_ground() ) {
        Sector::current()->add_object( std::make_shared<WaterDrop>(bbox.p1, get_water_sprite(), physic.get_velocity()) );
        remove_me();
        break;
      }
    } break;
    case STATE_GROUND_MELTING:
      is_active_flag = false;
      movement = physic.get_movement(elapsed_time);
      if ( sprite->animation_done() ) {
        remove_me();
      }
      break;
    case STATE_INSIDE_MELTING: {
      is_active_flag = false;
      movement = physic.get_movement(elapsed_time);
      if ( on_ground() && sprite->animation_done() ) {
        sprite->set_action(dir == LEFT ? "gear-left" : "gear-right", 1);
        set_state(STATE_GEAR);
      }
      int pa = graphicsRandom.rand(0,3);
      float px = graphicsRandom.randf(bbox.p1.x, bbox.p2.x);
      float py = graphicsRandom.randf(bbox.p1.y, bbox.p2.y);
      Vector ppos = Vector(px, py);
      Sector::current()->add_object(std::make_shared<SpriteParticle>(get_water_sprite(), "particle_" + std::to_string(pa),
                                                                     ppos, ANCHOR_MIDDLE,
                                                                     Vector(0, 0), Vector(0, 100 * Sector::current()->get_gravity()),
                                                                     LAYER_OBJECTS-1));
    } break;
    case STATE_FALLING:
      is_active_flag = false;
      movement = physic.get_movement(elapsed_time);
      break;
  }

  on_ground_flag = false;
}

void
BadGuy::save(Writer& writer) {
  MovingSprite::save(writer);
  if(dir == LEFT) {
    writer.write("direction", "left", false);
  } else {
    writer.write("direction", "right", false);
  }
  if(!dead_script.empty()) {
    writer.write("dead-script", dead_script, false);
  }
}

Direction
BadGuy::str2dir( std::string dir_str ) const
{
  if( dir_str == "auto" )
    return AUTO;
  if( dir_str == "left" )
    return LEFT;
  if( dir_str == "right" )
    return RIGHT;

  //default to "auto"
  log_warning << "Badguy::str2dir: unknown direction \"" << dir_str << "\"" << std::endl;;
  return AUTO;
}

void
BadGuy::initialize()
{
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
BadGuy::active_update(float elapsed_time)
{
  movement = physic.get_movement(elapsed_time);
  if(frozen)
    sprite->stop_animation();
}

void
BadGuy::inactive_update(float )
{
}

void
BadGuy::collision_tile(uint32_t tile_attributes)
{
  // Don't kill badguys that have already been killed
  if (!is_active()) return;

  if(tile_attributes & Tile::WATER && !is_in_water())
  {
    in_water = true;
    SoundManager::current()->play("sounds/splash.ogg", get_pos());
  }
  if(!(tile_attributes & Tile::WATER) && is_in_water())
  {
    in_water = false;
  }

  if(tile_attributes & Tile::HURTS) {
    if (tile_attributes & Tile::FIRE) {
      if (is_flammable()) ignite();
    }
    else if (tile_attributes & Tile::ICE) {
      if (is_freezable()) freeze();
    }
    else {
      kill_fall();
    }
  }
}

HitResponse
BadGuy::collision(GameObject& other, const CollisionHit& hit)
{
  if (!is_active()) return ABORT_MOVE;

  BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
  if(badguy && badguy->is_active() && badguy->get_group() == COLGROUP_MOVING) {

    /* Badguys don't let badguys squish other badguys. It's bad. */
#if 0
    // hit from above?
    if (badguy->get_bbox().p2.y < (bbox.p1.y + 16)) {
      if(collision_squished(*badguy)) {
        return ABORT_MOVE;
      }
    }
#endif

    return collision_badguy(*badguy, hit);
  }

  Player* player = dynamic_cast<Player*> (&other);
  if(player) {

    // hit from above?
    if (player->get_bbox().p2.y < (bbox.p1.y + 16)) {
      if(player->is_stone()) {
        kill_fall();
        return FORCE_MOVE;
      }
      if(collision_squished(*player)) {
        return FORCE_MOVE;
      }
    }

    if(player->is_stone()) {
      collision_solid(hit);
      return FORCE_MOVE;
    }

    return collision_player(*player, hit);
  }

  Bullet* bullet = dynamic_cast<Bullet*> (&other);
  if(bullet)
    return collision_bullet(*bullet, hit);

  return FORCE_MOVE;
}

void
BadGuy::collision_solid(const CollisionHit& hit)
{
  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
  update_on_ground_flag(hit);
}

HitResponse
BadGuy::collision_player(Player& player, const CollisionHit& )
{
  if(player.is_invincible()) {
    kill_fall();
    return ABORT_MOVE;
  }

  //TODO: unfreeze timer
  if(frozen)
    //unfreeze();
    return FORCE_MOVE;

  player.kill(false);
  return FORCE_MOVE;
}

HitResponse
BadGuy::collision_badguy(BadGuy& , const CollisionHit& )
{
  return FORCE_MOVE;
}

bool
BadGuy::collision_squished(GameObject& object)
{
  // frozen badguys can be killed with butt-jump
  if(frozen)
  {
    Player* player = dynamic_cast<Player*>(&object);
    if(player && (player->does_buttjump)) {
      player->bounce(*this);
      kill_fall();
      return true;
    }
  }
    return false;
}

HitResponse
BadGuy::collision_bullet(Bullet& bullet, const CollisionHit& hit)
{
  if (is_frozen()) {
    if(bullet.get_type() == FIRE_BONUS) {
      // fire bullet thaws frozen badguys
      unfreeze();
      bullet.remove_me();
      return ABORT_MOVE;
    } else {
      // other bullets ricochet
      bullet.ricochet(*this, hit);
      return FORCE_MOVE;
    }
  }
  else if (is_ignited()) {
    if(bullet.get_type() == ICE_BONUS) {
      // ice bullets extinguish ignited badguys
      extinguish();
      bullet.remove_me();
      return ABORT_MOVE;
    } else {
      // other bullets are absorbed by ignited badguys
      bullet.remove_me();
      return FORCE_MOVE;
    }
  }
  else if(bullet.get_type() == FIRE_BONUS && is_flammable()) {
    // fire bullets ignite flammable badguys
    ignite();
    bullet.remove_me();
    return ABORT_MOVE;
  }
  else if(bullet.get_type() == ICE_BONUS && is_freezable()) {
    // ice bullets freeze freezable badguys
    freeze();
    bullet.remove_me();
    return ABORT_MOVE;
  }
  else {
    // in all other cases, bullets ricochet
    bullet.ricochet(*this, hit);
    return FORCE_MOVE;
  }
}

void
BadGuy::kill_squished(GameObject& object)
{
  if (!is_active()) return;

  SoundManager::current()->play("sounds/squish.wav", get_pos());
  physic.enable_gravity(true);
  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
  set_state(STATE_SQUISHED);
  set_group(COLGROUP_MOVING_ONLY_STATIC);
  Player* player = dynamic_cast<Player*>(&object);
  if (player) {
    player->bounce(*this);
  }

  // start dead-script
  run_dead_script();
}

void
BadGuy::kill_fall()
{
  if (!is_active()) return;

  if (frozen) {
    SoundManager::current()->play("sounds/brick.wav");
    Vector pr_pos;
    float cx = bbox.get_width() / 2;
    float cy = bbox.get_height() / 2;
    for (pr_pos.x = 0; pr_pos.x < bbox.get_width(); pr_pos.x += 16) {
      for (pr_pos.y = 0; pr_pos.y < bbox.get_height(); pr_pos.y += 16) {
        Vector speed = Vector((pr_pos.x - cx) * 8, (pr_pos.y - cy) * 8 + 100);
        Sector::current()->add_object(
          std::make_shared<BrokenBrick>(sprite->clone(), bbox.p1 + pr_pos, speed));
      }
    }
    // start dead-script
    run_dead_script();
    remove_me();
  } else {
    SoundManager::current()->play("sounds/fall.wav", get_pos());
    physic.set_velocity_y(0);
    physic.set_acceleration_y(0);
    physic.enable_gravity(true);
    set_state(STATE_FALLING);

    // Set the badguy layer to be the foremost, so that
    // this does not reveal secret tilemaps:
    layer = Sector::current()->get_foremost_layer() + 1;
    // start dead-script
    run_dead_script();
  }

}

void
BadGuy::run_dead_script()
{
  if (countMe)
    Sector::current()->get_level()->stats.badguys++;

  countMe = false;

  // start dead-script
  if(!dead_script.empty()) {
    std::istringstream stream(dead_script);
    Sector::current()->run_script(stream, "dead-script");
  }
}

void
BadGuy::set_state(State state_)
{
  if(this->state == state_)
    return;

  State laststate = this->state;
  this->state = state_;
  switch(state_) {
    case STATE_BURNING:
      state_timer.start(BURN_TIME);
      break;
    case STATE_SQUISHED:
      state_timer.start(SQUISH_TIME);
      break;
    case STATE_GEAR:
      state_timer.start(GEAR_TIME);
      break;
    case STATE_ACTIVE:
      set_group(colgroup_active);
      //bbox.set_pos(start_position);
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
BadGuy::is_offscreen() const
{
  Vector dist;
  if (Editor::is_active()) {
    auto cam = Sector::current()->camera;
    dist = cam->get_center() - bbox.get_middle();
  }
  auto player = get_nearest_player();
  if (!player)
    return false;
  if(!Editor::is_active()) {
    dist = player->get_bbox().get_middle() - bbox.get_middle();
  }
  // In SuperTux 0.1.x, Badguys were activated when Tux<->Badguy center distance was approx. <= ~668px
  // This doesn't work for wide-screen monitors which give us a virt. res. of approx. 1066px x 600px
  if ((fabsf(dist.x) <= X_OFFSCREEN_DISTANCE) && (fabsf(dist.y) <= Y_OFFSCREEN_DISTANCE)) {
    return false;
  }
  return true;
}

void
BadGuy::try_activate()
{
  // Don't activate if player is dying
  Player* player = get_nearest_player();
  if (!player) return;

  if (!is_offscreen()) {
    set_state(STATE_ACTIVE);
    if (!is_initialized) {

      // if starting direction was set to AUTO, this is our chance to re-orient the badguy
      if (start_dir == AUTO) {
        Player* player_ = get_nearest_player();
        if (player_ && (player_->get_bbox().p1.x > bbox.p2.x)) {
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

bool
BadGuy::might_fall(int height) const
{
  // make sure we check for at least a 1-pixel fall
  assert(height > 0);

  float x1;
  float x2;
  float y1 = bbox.p2.y + 1;
  float y2 = bbox.p2.y + 1 + height;
  if (dir == LEFT) {
    x1 = bbox.p1.x - 1;
    x2 = bbox.p1.x;
  } else {
    x1 = bbox.p2.x;
    x2 = bbox.p2.x + 1;
  }
  return Sector::current()->is_free_of_statics(Rectf(x1, y1, x2, y2));
}

Player*
BadGuy::get_nearest_player() const
{
  return Sector::current()->get_nearest_player(bbox);
}

void
BadGuy::update_on_ground_flag(const CollisionHit& hit)
{
  if (hit.bottom) {
    on_ground_flag = true;
    floor_normal = hit.slope_normal;
  }
}

bool
BadGuy::on_ground() const
{
  return on_ground_flag;
}

bool
BadGuy::is_active() const
{
  return is_active_flag;
}

Vector
BadGuy::get_floor_normal() const
{
  return floor_normal;
}

void
BadGuy::freeze()
{
  set_group(COLGROUP_MOVING_STATIC);
  frozen = true;

  if(sprite->has_action("iced-left"))
    sprite->set_action(dir == LEFT ? "iced-left" : "iced-right", 1);
  // when the sprite doesn't have separate actions for left and right, it tries to use an universal one.
  else
  {
    if(sprite->has_action("iced"))
      sprite->set_action("iced", 1);
      // when no iced action exists, default to shading badguy blue
    else
    {
      sprite->set_color(Color(0.60, 0.72, 0.88f));
      sprite->stop_animation();
    }
  }
}

void
BadGuy::unfreeze()
{
  set_group(colgroup_active);
  frozen = false;

  // restore original color if needed
  if((!sprite->has_action("iced-left")) && (!sprite->has_action("iced")) )
  {
    sprite->set_color(Color(1.00, 1.00, 1.00f));
    sprite->set_animation_loops();
  }
}

bool
BadGuy::is_freezable() const
{
  return false;
}

bool
BadGuy::is_frozen() const
{
  return frozen;
}

bool
BadGuy::is_in_water() const
{
  return in_water;
}

void
BadGuy::ignite()
{
  if (!is_flammable() || ignited) {
    return;
  }

  physic.enable_gravity(true);
  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
  set_group(COLGROUP_MOVING_ONLY_STATIC);
  sprite->stop_animation();
  ignited = true;

  if (sprite->has_action("melting-left")) {

    // melt it!
    if (sprite->has_action("ground-melting-left") && on_ground()) {
      sprite->set_action(dir == LEFT ? "ground-melting-left" : "ground-melting-right", 1);
      SoundManager::current()->play("sounds/splash.ogg", get_pos());
      set_state(STATE_GROUND_MELTING);
    } else {
      sprite->set_action(dir == LEFT ? "melting-left" : "melting-right", 1);
      SoundManager::current()->play("sounds/sizzle.ogg", get_pos());
      set_state(STATE_MELTING);
    }

    run_dead_script();

  } else if (sprite->has_action("burning-left")) {
    // burn it!
    glowing = true;
    SoundManager::current()->play("sounds/fire.ogg", get_pos());
    sprite->set_action(dir == LEFT ? "burning-left" : "burning-right", 1);
    set_state(STATE_BURNING);
    run_dead_script();
  } else if (sprite->has_action("inside-melting-left")) {
    // melt it inside!
    SoundManager::current()->play("sounds/splash.ogg", get_pos());
    sprite->set_action(dir == LEFT ? "inside-melting-left" : "inside-melting-right", 1);
    set_state(STATE_INSIDE_MELTING);
    run_dead_script();
  } else {
    // Let it fall off the screen then.
    kill_fall();
  }
}

void
BadGuy::extinguish()
{
}

bool
BadGuy::is_flammable() const
{
  return true;
}

bool
BadGuy::is_ignited() const
{
  return ignited;
}

void
BadGuy::set_colgroup_active(CollisionGroup group_)
{
  this->colgroup_active = group_;
  if (state == STATE_ACTIVE) set_group(group_);
}

/* EOF */
