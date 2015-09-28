//  SuperTux BadGuy GoldBomb - a bomb that throws up coins when exploding
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "audio/sound_manager.hpp"
#include "badguy/goldbomb.hpp"
#include "object/coin_explode.hpp"
#include "object/explosion.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "util/reader.hpp"

GoldBomb::GoldBomb(const Reader& reader) :
  WalkingBadguy(reader, "images/creatures/gold_bomb/gold_bomb.sprite", "left", "right"),
  tstate(STATE_NORMAL),
  grabbed(false),
  grabber(NULL),
  ticking()
{
  walk_speed = 80;
  max_drop_height = 16;

  //Prevent stutter when Tux jumps on Gold Bomb
  SoundManager::current()->preload("sounds/explosion.wav");

  //Check if we need another sprite
  if( !reader.get( "sprite", sprite_name ) ){
    return;
  }
  if( sprite_name == "" ){
    sprite_name = "images/creatures/gold_bomb/gold_bomb.sprite";
    return;
  }
  //Replace sprite
  sprite = SpriteManager::current()->create( sprite_name );
}

void
GoldBomb::collision_solid(const CollisionHit& hit)
{
  if(tstate == STATE_TICKING) {
    if(hit.bottom) {
      physic.set_velocity_y(0);
      physic.set_velocity_x(0);
    }else if (hit.left || hit.right)
      physic.set_velocity_x(-physic.get_velocity_x());
    else if (hit.top)
      physic.set_velocity_y(0);
    update_on_ground_flag(hit);
    return;
  }
  WalkingBadguy::collision_solid(hit);
}

HitResponse
GoldBomb::collision(GameObject& object, const CollisionHit& hit)
{
  if(tstate == STATE_TICKING)
    return ABORT_MOVE;
  if(grabbed)
    return FORCE_MOVE;
  return WalkingBadguy::collision(object, hit);
}

HitResponse
GoldBomb::collision_player(Player& player, const CollisionHit& hit)
{
  if(tstate == STATE_TICKING)
    return ABORT_MOVE;
  if(grabbed)
    return FORCE_MOVE;
  return WalkingBadguy::collision_player(player, hit);
}

HitResponse
GoldBomb::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  if(tstate == STATE_TICKING)
    return ABORT_MOVE;
  return WalkingBadguy::collision_badguy(badguy, hit);
}

bool
GoldBomb::collision_squished(GameObject& object)
{
  Player* player = dynamic_cast<Player*>(&object);
  if(player && player->is_invincible()) {
    player->bounce(*this);
    kill_fall();
    return true;
  }
  if(is_valid() && tstate == STATE_NORMAL) {
    tstate = STATE_TICKING;
    frozen = false;
    set_action(dir == LEFT ? "ticking-left" : "ticking-right", 1);
    physic.set_velocity_x(0);

    if (player)
      player->bounce(*this);

    ticking = SoundManager::current()->create_sound_source("sounds/fizz.wav");
    ticking->set_position(get_pos());
    ticking->set_looping(true);
    ticking->set_gain(2.0);
    ticking->set_reference_distance(32);
    ticking->play();
  }
  return true;
}

void
GoldBomb::active_update(float elapsed_time)
{
  if(tstate == STATE_TICKING) {
    if (on_ground()) physic.set_velocity_x(0);
    ticking->set_position(get_pos());
    if(sprite->animation_done()) {
      kill_fall();
    }
    else if (!grabbed) {
      movement = physic.get_movement(elapsed_time);
    }
    return;
  }
  if(grabbed)
    return;
  WalkingBadguy::active_update(elapsed_time);
}

void
GoldBomb::kill_fall()
{
  if(tstate == STATE_TICKING)
    ticking->stop();

  // Make the player let go before we explode, otherwise the player is holding
  // an invalid object. There's probably a better way to do this than in the
  // GoldBomb class.
  if (grabber != NULL) {
    Player* player = dynamic_cast<Player*>(grabber);

    if (player)
      player->stop_grabbing();
  }

  if(is_valid()) {
    remove_me();
    Sector::current()->add_object(std::make_shared<Explosion>(get_bbox().get_middle()));
    Sector::current()->add_object(std::make_shared<CoinExplode>(get_pos() + Vector (0, -40)));
  }

  run_dead_script();
}

void
GoldBomb::ignite()
{
  kill_fall();
}

void
GoldBomb::grab(MovingObject& object, const Vector& pos, Direction dir_)
{
  if(tstate == STATE_TICKING){
    movement = pos - get_pos();
    this->dir = dir_;

    // We actually face the opposite direction of Tux here to make the fuse more
    // visible instead of hiding it behind Tux
    sprite->set_action_continued(dir == LEFT ? "ticking-right" : "ticking-left");
    set_colgroup_active(COLGROUP_DISABLED);
    grabbed = true;
    grabber = &object;
  }
  else if(frozen){
    movement = pos - get_pos();
    this->dir = dir_;
    sprite->set_action(dir_ == LEFT ? "iced-left" : "iced-right");
    set_colgroup_active(COLGROUP_DISABLED);
    grabbed = true;
  }
}

void
GoldBomb::ungrab(MovingObject& object, Direction dir_)
{
  int toss_velocity_x = 0;
  int toss_velocity_y = 0;
  Player* player = dynamic_cast<Player*> (&object);

  // toss upwards
  if(dir_ == UP)
    toss_velocity_y += -500;

  // toss to the side when moving sideways
  if(player && player->physic.get_velocity_x()*(dir_ == LEFT ? -1 : 1) > 1) {
    toss_velocity_x += (dir_ == LEFT) ? -200 : 200;
    toss_velocity_y = (toss_velocity_y < -200) ? toss_velocity_y : -200;
    // toss farther when running
    if(player && player->physic.get_velocity_x()*(dir_ == LEFT ? -1 : 1) > 200)
      toss_velocity_x += player->physic.get_velocity_x()-(190*(dir_ == LEFT ? -1 : 1));
  }
  log_warning << toss_velocity_x << toss_velocity_y << std::endl;////

  //set_pos(object.get_pos() + Vector((dir_ == LEFT ? -33 : 33), get_bbox().get_height()*0.66666 - 32));
  physic.set_velocity(toss_velocity_x, toss_velocity_y);
  set_colgroup_active(COLGROUP_MOVING);
  grabbed = false;
}

void
GoldBomb::freeze()
{
  if(tstate == STATE_NORMAL){
    WalkingBadguy::freeze();
  }
}

bool
GoldBomb::is_freezable() const
{
  return true;
}

bool
GoldBomb::is_portable() const
{
  return (frozen || (tstate == STATE_TICKING));
}


ObjectSettings
GoldBomb::get_settings() {
  ObjectSettings result(_("Golden bomb"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Death script"), &dead_script));
  return result;
}

/* EOF */
