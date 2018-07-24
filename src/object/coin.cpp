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
#include "audio/sound_source.hpp"
#include "editor/editor.hpp"
#include "object/bouncy_coin.hpp"
#include "object/player.hpp"
#include "object/tilemap.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

Coin::Coin(const Vector& pos)
  : MovingSprite(pos, "images/objects/coin/coin.sprite", LAYER_OBJECTS - 1, COLGROUP_TOUCHABLE),
    PathObject(),
    offset(),
    from_tilemap(false),
    add_path(false),
    physic(),
    collect_script()
{
  SoundManager::current()->preload("sounds/coin.wav");
}

Coin::Coin(const Vector& pos, TileMap* tilemap)
  : MovingSprite(pos, "images/objects/coin/coin.sprite", LAYER_OBJECTS - 1, COLGROUP_TOUCHABLE),
    PathObject(*tilemap),
    offset(),
    from_tilemap(true),
    add_path(false),
    physic(),
    collect_script()
{
  if(walker.get()) {
    Vector v = path->get_base();
    offset = pos - v;
  }

  SoundManager::current()->preload("sounds/coin.wav");
}

Coin::Coin(const ReaderMapping& reader)
  : MovingSprite(reader, "images/objects/coin/coin.sprite", LAYER_OBJECTS - 1, COLGROUP_TOUCHABLE),
    PathObject(),
    offset(),
    from_tilemap(false),
    add_path(false),
    physic(),
    collect_script()
{
  ReaderMapping path_mapping;
  if (reader.get("path", path_mapping)) {
    path.reset(new Path());
    path->read(path_mapping);
    walker.reset(new PathWalker(path.get()));
    Vector v = path->get_base();
    set_pos(v);
  }

  reader.get("collect-script", collect_script, "");

  SoundManager::current()->preload("sounds/coin.wav");
}

void
Coin::save(Writer& writer) {
  MovingSprite::save(writer);
  if (path) {
    path->save(writer);
  }
}

void
Coin::update(float elapsed_time)
{
  // if we have a path to follow, follow it
  if (walker.get()) {
    Vector v = from_tilemap ? offset + walker->get_pos() : walker->advance(elapsed_time);
    if (path->is_valid()) {
      if (Editor::is_active()) {
        set_pos(v);
      } else {
        movement = v - get_pos();
      }
    }
  }
}

void
Coin::collect()
{
  static Timer sound_timer;
  static int pitch_one = 128;
  static float last_pitch = 1;
  float pitch = 1;

  int tile = static_cast<int>(get_pos().y / 32);

  if (!sound_timer.started()) {
    pitch_one = tile;
    pitch = 1;
    last_pitch = 1;
  } else if (sound_timer.get_timegone() < 0.02) {
    pitch = last_pitch;
  } else {
    switch ((pitch_one - tile) % 7) {
      case -6:
        pitch = 1.f/2;
        break;
      case -5:
        pitch = 5.f/8;
        break;
      case -4:
        pitch = 4.f/6;
        break;
      case -3:
        pitch = 3.f/4;
        break;
      case -2:
        pitch = 5.f/6;
        break;
      case -1:
        pitch = 9.f/10;
        break;
      case 0:
        pitch = 1.f;
        break;
      case 1:
        pitch = 9.f/8;
        break;
      case 2:
        pitch = 5.f/4;
        break;
      case 3:
        pitch = 4.f/3;
        break;
      case 4:
        pitch = 3.f/2;
        break;
      case 5:
        pitch = 5.f/3;
        break;
      case 6:
        pitch = 9.f/5;
        break;
    }
    last_pitch = pitch;
  }
  sound_timer.start(1);

  std::unique_ptr<SoundSource> soundSource = SoundManager::current()->create_sound_source("sounds/coin.wav");
  soundSource->set_position(get_pos());
  soundSource->set_pitch(pitch);
  soundSource->play();
  SoundManager::current()->manage_source(std::move(soundSource));

  auto sector = Sector::current();
  sector->player->get_status()->add_coins(1, false);
  sector->add_object(std::make_shared<BouncyCoin>(get_pos(), false, get_sprite_name()));
  sector->get_level()->stats.coins++;
  remove_me();

  if(!collect_script.empty()) {
    sector->run_script(collect_script, "collect-script");
  }
}

HitResponse
Coin::collision(GameObject& other, const CollisionHit& )
{
  auto player = dynamic_cast<Player*>(&other);
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
  SoundManager::current()->preload("sounds/coin2.ogg");
  set_group(COLGROUP_MOVING);
  physic.set_velocity(init_velocity);
}

HeavyCoin::HeavyCoin(const ReaderMapping& reader)
  : Coin(reader),
  physic()
{
  physic.enable_gravity(true);
  SoundManager::current()->preload("sounds/coin2.ogg");
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
      SoundManager::current()->play("sounds/coin2.ogg");
    if(physic.get_velocity_y() > 200) {// lets some coins bounce
      physic.set_velocity_y(-99);
    } else {
      physic.set_velocity_y(0);
      physic.set_velocity_x(0);
    }
  }
  if(hit.right || hit.left) {
    if(physic.get_velocity_x() > clink_threshold || physic.get_velocity_x() < clink_threshold)
      SoundManager::current()->play("sounds/coin2.ogg");
    physic.set_velocity_x(-physic.get_velocity_x());
  }
  if(hit.top) {
    if(physic.get_velocity_y() < clink_threshold)
      SoundManager::current()->play("sounds/coin2.ogg");
    physic.set_velocity_y(-physic.get_velocity_y());
  }
}

void
Coin::move_to(const Vector& pos)
{
  Vector shift = pos - bbox.p1;
  if (path) {
    path->move_by(shift);
  }
  set_pos(pos);
}

ObjectSettings
Coin::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  add_path = walker.get() && path->is_valid();
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Following path"), &add_path));

  if (walker.get() && path->is_valid()) {
    result.options.push_back( Path::get_mode_option(&path->mode) );
  }

  result.options.push_back( ObjectOption(MN_SCRIPT, _("Collect script"),
                                         &collect_script, "collect-script"));

  return result;
}

void
Coin::after_editor_set()
{
  MovingSprite::after_editor_set();

  if (walker.get() && path->is_valid()) {
    if (!add_path) {
      path->nodes.clear();
    }
  } else {
    if (add_path) {
      path.reset(new Path(bbox.p1));
      walker.reset(new PathWalker(path.get()));
    }
  }
}

ObjectSettings
HeavyCoin::get_settings()
{
  auto result = MovingSprite::get_settings();
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Collect script"),
                                         &collect_script, "collect-script"));
  return result;
}

void
HeavyCoin::after_editor_set()
{
  MovingSprite::after_editor_set();
}

/* EOF */
