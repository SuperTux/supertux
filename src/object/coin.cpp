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

#include "object/coin.hpp"

#include "audio/sound_manager.hpp"
#include "util/reader.hpp"
#include "object/bouncy_coin.hpp"
#include "object/player.hpp"
#include "object/tilemap.hpp"
#include "supertux/level.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

Coin::Coin(const Vector& pos)
  : MovingSprite(pos, "images/objects/coin/coin.sprite", LAYER_TILES, COLGROUP_TOUCHABLE),
    path(),
    walker(),
    offset(),
    from_tilemap(false),
    physic()
{
  sound_manager->preload("sounds/coin.wav");
}

Coin::Coin(const Vector& pos, TileMap* tilemap)
  : MovingSprite(pos, "images/objects/coin/coin.sprite", LAYER_TILES, COLGROUP_TOUCHABLE),
    path(boost::shared_ptr<Path>(tilemap->get_path())),
    walker(boost::shared_ptr<PathWalker>(tilemap->get_walker())),
    offset(),
    from_tilemap(true),
    physic()
{
  if(walker.get()) {
    Vector v = path->get_base();
    offset = pos - v;
  }

  sound_manager->preload("sounds/coin.wav");
}

Coin::Coin(const Reader& reader)
  : MovingSprite(reader, "images/objects/coin/coin.sprite", LAYER_TILES, COLGROUP_TOUCHABLE),
    path(),
    walker(),
    offset(),
    from_tilemap(false),
    physic()
{
  const lisp::Lisp* pathLisp = reader.get_lisp("path");
  if (pathLisp) {
    path.reset(new Path());
    path->read(*pathLisp);
    walker.reset(new PathWalker(path.get()));
    Vector v = path->get_base();
    set_pos(v);
  }

  sound_manager->preload("sounds/coin.wav");
}

void
Coin::update(float elapsed_time)
{
  // if we have a path to follow, follow it
  if (walker.get()) {
    Vector v = from_tilemap ? offset + walker->get_pos() : walker->advance(elapsed_time);
    movement = v - get_pos();
  }
}

void
Coin::collect()
{
  // TODO: commented out musical code. Maybe fork this for a special "MusicalCoin" object?
  /*
    static Timer sound_timer;
    static int pitch_one = 128;
    static float last_pitch = 1;
    float pitch = 1;

    int tile = static_cast<int>(get_pos().y / 32);

    if (!sound_timer.started()) {
    pitch_one = tile;
    pitch = 1;
    last_pitch = 1;
    }
    else if (sound_timer.get_timegone() < 0.02) {
    pitch = last_pitch;
    }
    else
    {
    switch ((pitch_one - tile) % 7) {
    case -6:
    pitch = 1.0/2;
    break;
    case -5:
    pitch = 5.0/8;
    break;
    case -4:
    pitch = 4.0/6;
    break;
    case -3:
    pitch = 3.0/4;
    break;
    case -2:
    pitch = 5.0/6;
    break;
    case -1:
    pitch = 9.0/10;
    break;
    case 0:
    pitch = 1.0;
    break;
    case 1:
    pitch = 9.0/8;
    break;
    case 2:
    pitch = 5.0/4;
    break;
    case 3:
    pitch = 4.0/3;
    break;
    case 4:
    pitch = 3.0/2;
    break;
    case 5:
    pitch = 5.0/3;
    break;
    case 6:
    pitch = 9.0/5;
    break;
    }
    last_pitch = pitch;
    }
    sound_timer.start(1);

    SoundSource* soundSource = sound_manager->create_sound_source("sounds/coin.wav");
    soundSource->set_position(get_pos());
    soundSource->set_pitch(pitch);
    soundSource->play();
    sound_manager->manage_source(soundSource);
  */
  Sector::current()->player->get_status()->add_coins(1);
  Sector::current()->add_object(new BouncyCoin(get_pos()));
  Sector::current()->get_level()->stats.coins++;
  remove_me();
}

HitResponse
Coin::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*>(&other);
  if(player == 0)
    return ABORT_MOVE;

  collect();
  return ABORT_MOVE;
}

/* The following defines a coin subject to gravity */
HeavyCoin::HeavyCoin(const Vector& pos, const Vector& init_velocity)
  : Coin(pos),
  physic()
{
  physic.enable_gravity(true);
  sound_manager->preload("sounds/coin2.ogg");
  set_group(COLGROUP_MOVING);
  physic.set_velocity(init_velocity);
}

HeavyCoin::HeavyCoin(const Reader& reader)
  : Coin(reader),
  physic()
{
  physic.enable_gravity(true);
  sound_manager->preload("sounds/coin2.ogg");
  set_group(COLGROUP_MOVING);
}

void
HeavyCoin::update(float elapsed_time)
{
  // enable physics
  movement = physic.get_movement(elapsed_time);
}

void
HeavyCoin::collision_solid(const CollisionHit& hit)
{
  int clink_threshold = 100; // sets the minimum speed needed to result in collision noise
  //TODO: colliding HeavyCoins should have their own unique sound

  if(hit.bottom) {
    if(physic.get_velocity_y() > clink_threshold)
      sound_manager->play("sounds/coin2.ogg");
    if(physic.get_velocity_y() > 200) {// lets some coins bounce
      physic.set_velocity_y(-99);
    }else{
      physic.set_velocity_y(0);
      physic.set_velocity_x(0);
    }
  }
  if(hit.right || hit.left) {
    if(physic.get_velocity_x() > clink_threshold || physic.get_velocity_x() < clink_threshold)
      sound_manager->play("sounds/coin2.ogg");
    physic.set_velocity_x(-physic.get_velocity_x());
  }
  if(hit.top) {
    if(physic.get_velocity_y() < clink_threshold)
      sound_manager->play("sounds/coin2.ogg");
    physic.set_velocity_y(-physic.get_velocity_y());
  }
}

/* EOF */
