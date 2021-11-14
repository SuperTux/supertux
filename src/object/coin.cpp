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
#include "util/writer.hpp"

Coin::Coin(const Vector& pos) :
  MovingSprite(pos, "images/objects/coin/coin.sprite", LAYER_OBJECTS - 1, COLGROUP_TOUCHABLE),
  PathObject(),
  m_offset(0.0f, 0.0f),
  m_from_tilemap(false),
  m_add_path(false),
  m_physic(),
  m_collect_script(),
  m_starting_node(0)
{
  SoundManager::current()->preload("sounds/coin.wav");
}

Coin::Coin(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/coin/coin.sprite", LAYER_OBJECTS - 1, COLGROUP_TOUCHABLE),
  PathObject(),
  m_offset(0.0f, 0.0f),
  m_from_tilemap(false),
  m_add_path(false),
  m_physic(),
  m_collect_script(),
  m_starting_node(0)
{
  reader.get("starting-node", m_starting_node, 0.f);

  init_path(reader, true);

  reader.get("collect-script", m_collect_script, "");

  SoundManager::current()->preload("sounds/coin.wav");
}

void
Coin::finish_construction()
{
  if (get_path())
  {
    Vector v = get_path()->get_base();
    set_pos(v);
  }

  m_add_path = get_walker() && get_path() && get_path()->is_valid();
}

void
Coin::update(float dt_sec)
{
  // if we have a path to follow, follow it
  if (get_walker()) {
    Vector v(0.0f, 0.0f);
    if (m_from_tilemap)
    {
      v = m_offset + get_walker()->get_pos();
    }
    else
    {
      get_walker()->update(dt_sec);
      v = get_walker()->get_pos();
    }

    if (get_path()->is_valid()) {
      m_col.set_movement(v - get_pos());
    }
  }
}

void
Coin::editor_update()
{
  if (get_walker()) {
    if (m_from_tilemap) {
      set_pos(m_offset + get_walker()->get_pos());
    } else {
      set_pos(get_walker()->get_pos());
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
  } else if (sound_timer.get_timegone() < 0.02f) {
    pitch = last_pitch;
  } else {
    switch ((pitch_one - tile) % 7) {
      case -6:
        pitch = 1.f/2;  // C
        break;
      case -5:
        pitch = 5.f/8;  // E
        break;
      case -4:
        pitch = 4.f/6;  // F
        break;
      case -3:
        pitch = 3.f/4;  // G
        break;
      case -2:
        pitch = 5.f/6;  // A
        break;
      case -1:
        pitch = 9.f/10;  // Bb
        break;
      case 0:
        pitch = 1.f;  // c
        break;
      case 1:
        pitch = 9.f/8;  // d
        break;
      case 2:
        pitch = 5.f/4;  // e
        break;
      case 3:
        pitch = 4.f/3;  // f
        break;
      case 4:
        pitch = 3.f/2;  // g
        break;
      case 5:
        pitch = 5.f/3;  // a
        break;
      case 6:
        pitch = 9.f/5;  // bb
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

  Sector::get().get_player().get_status().add_coins(1, false);
  Sector::get().add<BouncyCoin>(get_pos(), false, get_sprite_name());
  Sector::get().get_level().m_stats.increment_coins();
  remove_me();

  if (!m_collect_script.empty()) {
    Sector::get().run_script(m_collect_script, "collect-script");
  }
}

HitResponse
Coin::collision(GameObject& other, const CollisionHit& )
{
  auto player = dynamic_cast<Player*>(&other);
  if (player == nullptr)
    return ABORT_MOVE;
  if (m_col.get_bbox().contains(player->get_bbox().grown(-0.1f)))
    collect();
  return ABORT_MOVE;
}

/* The following defines a coin subject to gravity */
HeavyCoin::HeavyCoin(const Vector& pos, const Vector& init_velocity) :
  Coin(pos),
  m_physic(),
  m_last_hit()
{
  m_physic.enable_gravity(true);
  SoundManager::current()->preload("sounds/coin2.ogg");
  set_group(COLGROUP_MOVING);
  m_physic.set_velocity(init_velocity);
}

HeavyCoin::HeavyCoin(const ReaderMapping& reader) :
  Coin(reader),
  m_physic(),
  m_last_hit()
{
  m_physic.enable_gravity(true);
  SoundManager::current()->preload("sounds/coin2.ogg");
  set_group(COLGROUP_MOVING);
}

void
HeavyCoin::update(float dt_sec)
{
  // enable physics
  m_col.set_movement(m_physic.get_movement(dt_sec));
}

void
HeavyCoin::collision_solid(const CollisionHit& hit)
{
  float clink_threshold = 100.0f; // sets the minimum speed needed to result in collision noise
  //TODO: colliding HeavyCoins should have their own unique sound

  if (hit.bottom) {
    if (m_physic.get_velocity_y() > clink_threshold && !m_last_hit.bottom)
        SoundManager::current()->play("sounds/coin2.ogg");
    if (m_physic.get_velocity_y() > 200) {// lets some coins bounce
      m_physic.set_velocity_y(-99);
    } else {
      m_physic.set_velocity_y(0);
      m_physic.set_velocity_x(0);
    }
  }
  if (hit.right || hit.left) {
    if ((m_physic.get_velocity_x() > clink_threshold ||
         m_physic.get_velocity_x()< -clink_threshold) &&
         hit.right != m_last_hit.right && hit.left != m_last_hit.left)
      SoundManager::current()->play("sounds/coin2.ogg");
    m_physic.set_velocity_x(-m_physic.get_velocity_x());
  }
  if (hit.top) {
    if (m_physic.get_velocity_y() < -clink_threshold && !m_last_hit.top)
      SoundManager::current()->play("sounds/coin2.ogg");
    m_physic.set_velocity_y(-m_physic.get_velocity_y());
  }

  // Only make a sound if the coin wasn't hittin anything last frame (A coin
  // stuck in solid matter would flood the sound manager - see #1555 on GitHub)
  m_last_hit = hit;
}

void
Coin::move_to(const Vector& pos)
{
  Vector shift = pos - m_col.m_bbox.p1();
  if (get_path()) {
    get_path()->move_by(shift);
  }
  set_pos(pos);
}

ObjectSettings
Coin::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.add_path_ref(_("Path"), *this, get_path_ref(), "path-ref");
  m_add_path = get_walker() && get_path() && get_path()->is_valid();
  result.add_bool(_("Following path"), &m_add_path);

  if (get_walker() && get_path()->is_valid()) {
    result.add_walk_mode(_("Path Mode"), &get_path()->m_mode, {}, {});
    result.add_bool(_("Adapt Speed"), &get_path()->m_adapt_speed, {}, {});
    result.add_int(_("Starting Node"), &m_starting_node, "starting-node", 0, 0U);
  }

  result.add_script(_("Collect script"), &m_collect_script, "collect-script");

  result.reorder({"collect-script", "path-ref"});

  return result;
}

void
Coin::after_editor_set()
{
  MovingSprite::after_editor_set();

  if (get_walker() && get_path()->is_valid()) {
    if (!m_add_path) {
      get_path()->m_nodes.clear();
    }
  } else {
    if (m_add_path) {
      init_path_pos(m_col.m_bbox.p1());
    }
  }
}

ObjectSettings
HeavyCoin::get_settings()
{
  auto result = MovingSprite::get_settings();

  result.add_script(_("Collect script"), &m_collect_script, "collect-script");

  result.reorder({"collect-script", "sprite", "x", "y"});

  return result;
}

void
HeavyCoin::after_editor_set()
{
  MovingSprite::after_editor_set();
}

/* EOF */
