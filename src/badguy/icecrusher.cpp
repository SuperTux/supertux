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

#include "badguy/icecrusher.hpp"

#include <algorithm>
#include <math.h>
#include <string>

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "object/brick.hpp"
#include "object/coin.hpp"
#include "object/camera.hpp"
#include "object/particles.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

namespace {
  /* Maximum movement speed in pixels per LOGICAL_FPS */
  const float RECOVER_SPEED_NORMAL = -3.125f;
  const float RECOVER_SPEED_LARGE = -2.0f;
  const float DROP_ACTIVATION_DISTANCE = 4.0f;
  const float PAUSE_TIME_NORMAL = 0.5f;
  const float PAUSE_TIME_LARGE = 1.0f;
}

IceCrusher::IceCrusher(const ReaderMapping& reader) :
  MovingSprite(reader, "images/creatures/icecrusher/icecrusher.sprite", LAYER_OBJECTS, COLGROUP_MOVING_STATIC),
  m_state(IDLE),
  m_ic_size(NORMAL),
  m_start_position(get_bbox().p1()),
  m_physic(),
  m_cooldown_timer(0.0),
  m_sideways(),
  m_side_dir(),
  m_lefteye(),
  m_righteye(),
  m_whites()
{
  reader.get("sideways", m_sideways);
  // TODO: icecrusher hitting deserves its own sounds-
  // one for hitting the ground, one for hitting Tux
  SoundManager::current()->preload(not_ice() ? "sounds/thud.ogg" : "sounds/brick.wav");
  set_state(m_state, true);
  after_sprite_set();
}

HitResponse
IceCrusher::collision(GameObject& other, const CollisionHit& hit)
{
  auto player = dynamic_cast<Player*>(&other);

  // If the other object is the player, and the collision is at the
  // bottom of the ice crusher, hurt the player.
  if (player && hit.bottom && player->on_ground() && m_state == CRUSHING) {
    SoundManager::current()->play("sounds/brick.wav", get_pos());
    set_state(RECOVERING);
    if (player->is_invincible()) {
      return ABORT_MOVE;
    }
    player->kill(false);
    return FORCE_MOVE;
  }

  auto badguy = dynamic_cast<BadGuy*>(&other);
  if (badguy && m_state == CRUSHING) {
    badguy->kill_fall();
  }

  auto heavy_coin = dynamic_cast<HeavyCoin*>(&other);
  if (heavy_coin) {
    return ABORT_MOVE;
  }
  return FORCE_MOVE;
}

void
IceCrusher::collision_solid(const CollisionHit& hit)
{
  if (hit.left || hit.right)
    m_physic.set_velocity_x(0.f);
  if (hit.top || hit.bottom)
    m_physic.set_velocity_y(0.f);

  std::string crush_sound;
  if (not_ice())
    crush_sound = "sounds/thud.ogg";
  else
    crush_sound = "sounds/brick.wav";

  float shake_time = m_ic_size == LARGE ? 0.125f : 0.1f;
  float shake_x = (m_sideways ? m_ic_size == LARGE ? 16.f : 8.f : 0.f)*
    (m_side_dir == Direction::LEFT ? -1.f : 1.f);
  float shake_y = m_sideways ? 0.f : m_ic_size == LARGE ? 16.f : 8.f;


  switch (m_state) {
  case IDLE:
    break;
  case CRUSHING:
    if ((!m_sideways && ((m_flip == NO_FLIP && hit.bottom) || (m_flip != NO_FLIP && hit.top))) ||
      (m_sideways && ((m_side_dir == Direction::RIGHT && hit.right) ||
      (m_side_dir == Direction::LEFT && hit.left))))
    {
      SoundManager::current()->play(crush_sound, get_pos());
      Sector::get().get_camera().shake(shake_time, shake_x, shake_y);
      m_cooldown_timer = m_ic_size == LARGE ? PAUSE_TIME_LARGE : PAUSE_TIME_NORMAL;
      set_state(RECOVERING);

      // throw some particles
      for (int j = 0; j < 5; j++)
      {
        if (!m_sideways)
        {
          Sector::get().add<Particles>(
            Vector(m_col.m_bbox.get_right() - static_cast<float>(j) * 8.0f - 4.0f,
            (m_flip == NO_FLIP ? m_col.m_bbox.get_bottom() : m_col.m_bbox.get_top())),
            0, 90 + 10 * j, 140, 260, Vector(0, 500),
            1, Color(.6f, .6f, .6f), 4, 1.6f, LAYER_OBJECTS + 1);
          Sector::get().add<Particles>(
            Vector(m_col.m_bbox.get_left() + static_cast<float>(j) * 8.0f + 4.0f,
            (m_flip == NO_FLIP ? m_col.m_bbox.get_bottom() : m_col.m_bbox.get_top())),
            270 + 10 * j, 360, 140, 260, Vector(0, 500),
            1, Color(.6f, .6f, .6f), 4, 1.6f, LAYER_OBJECTS + 1);
        }
        else
        {
          int min_angle = m_side_dir == Direction::LEFT ? 0 : 270 + 10 * j;
          int max_angle = m_side_dir == Direction::LEFT ? 90 + 10 * j : 360;
          Sector::get().add<Particles>(
            Vector((m_side_dir == Direction::RIGHT ? m_col.m_bbox.get_right() : m_col.m_bbox.get_left()),
            (m_col.m_bbox.get_top())), min_angle, max_angle, 140, 260, Vector(0, 500),
            1, Color(.6f, .6f, .6f), 4, 1.6f, LAYER_OBJECTS + 1);
          Sector::get().add<Particles>(
            Vector((m_side_dir == Direction::RIGHT ? m_col.m_bbox.get_right() : m_col.m_bbox.get_left()),
            (m_col.m_bbox.get_bottom())), min_angle, max_angle, 140, 260, Vector(0, 500),
            1, Color(.6f, .6f, .6f), 4, 1.6f, LAYER_OBJECTS + 1);
        }
      }
    }
    if (hit.bottom)
      spawn_roots(Direction::DOWN);
    if (hit.left)
      spawn_roots(Direction::LEFT);
    if (hit.right)
      spawn_roots(Direction::RIGHT);
    break;
  default:
    log_debug << "IceCrusher in invalid state" << std::endl;
    break;
  }
}

void
IceCrusher::update(float dt_sec)
{
  Vector movement = m_physic.get_movement(dt_sec);
  m_col.set_movement(movement);
  m_col.propagate_movement(movement);
  if (m_cooldown_timer >= dt_sec)
  {
    m_cooldown_timer -= dt_sec;
    return;
  }
  else if (m_cooldown_timer != 0.0f)
  {
    dt_sec -= m_cooldown_timer;
    m_cooldown_timer = 0.0;
  }

  //because this game's physics are so broken, we have to create faux collisions with bricks

  for (auto& brick : Sector::get().get_objects_by_type<Brick>())
  {
    Rectf brickbox = get_bbox().grown(-1);
    brickbox.set_bottom(m_sideways ? get_bbox().get_bottom() - 1.f :
      m_flip == NO_FLIP ? get_bbox().get_bottom() + 9.f : get_bbox().get_bottom() - 1.f);
    brickbox.set_top(m_sideways ? get_bbox().get_top() + 1.f :
      m_flip != NO_FLIP ? get_bbox().get_top() - 9.f : get_bbox().get_top() + 1.f);
    brickbox.set_left((m_sideways && m_physic.get_velocity_x() < 0.f) ?
      get_bbox().get_left() - 9.f : get_bbox().get_left() + 1.f);
    brickbox.set_right((m_sideways && m_physic.get_velocity_x() > 0.f) ?
      get_bbox().get_right() + 9.f : get_bbox().get_right() - 1.f);

    if (brickbox.contains(brick.get_bbox()))
    {
      if (brick.get_class() != "heavy-brick")
      {
        brick.break_for_crusher(this);
      }
      else
      {
        if (is_big()) {
          brick.break_for_crusher(this);
        }
      }
    }
  }

  //determine whether side-crushers will go left or right

  if (auto* player = Sector::get().get_nearest_player(m_col.m_bbox))
  {
    const Rectf& player_bbox = player->get_bbox();
    if (m_state == IDLE) {
      m_side_dir = (player_bbox.get_middle().x > get_bbox().get_middle().x) ? Direction::RIGHT : Direction::LEFT;
    }
  }

  // handle blockage
  Rectf recover_box = get_bbox().grown(-1);
  if (!m_sideways)
  {
    recover_box.set_top(get_bbox().get_top() + (m_flip == NO_FLIP ? -1.f : 1.f));
    recover_box.set_bottom(get_bbox().get_bottom() + (m_flip == NO_FLIP ? -1.f : 1.f));
  }
  else
  {
    if (m_side_dir == Direction::LEFT)
    {
      recover_box.set_right(get_bbox().get_right() + 1.f);
      recover_box.set_left(get_bbox().get_left() + 1.f);
    }
    else
    {
      recover_box.set_right(get_bbox().get_right() - 1.f);
      recover_box.set_left(get_bbox().get_left() - 1.f);
    }
  }
  bool blocked = !Sector::get().is_free_of_statics(recover_box);

  //velocity for recovery speed
  float recover_x;
  float recover_y;

  if (!blocked)
  {
    recover_x = m_sideways ? m_side_dir == Direction::LEFT ? 160.f : -160.f : 0.f;
    recover_y = m_sideways ? 0.f : m_flip == NO_FLIP ? -160.f : 160.f;
  }
  else
  {
    recover_x = 0.f;
    recover_y = 0.f;
  }

  bool returned_down = m_flip == NO_FLIP && !m_sideways && get_bbox().get_top() <= m_start_position.y + 2.f;
  bool returned_up = m_flip != NO_FLIP && !m_sideways && get_bbox().get_top() >= m_start_position.y - 2.f;
  bool returned_left = m_sideways && m_side_dir == Direction::LEFT && get_bbox().get_left() >= m_start_position.x - 2.f;
  bool returned_right = m_sideways && m_side_dir == Direction::RIGHT && get_bbox().get_left() <= m_start_position.x + 2.f;

  //handle crusher states
  switch (m_state)
  {
  case IDLE:
    m_start_position = get_pos();
    if (found_victim())
    {
      set_state(CRUSHING);
    }
    break;
  case CRUSHING:
    if (!m_sideways)
    {
      if (m_flip == NO_FLIP)
      {
        if (m_physic.get_velocity_y() > 700.f)
          m_physic.set_velocity_y(700.f);
        else
          m_physic.set_velocity_y(m_physic.get_velocity_y() + 15.f);
      }
      else
      {
        if (m_physic.get_velocity_y() < -700.f)
          m_physic.set_velocity_y(-700.f);
        else
          m_physic.set_velocity_y(m_physic.get_velocity_y() - 15.f);
      }
    }
    else
    {
      m_physic.set_velocity((m_physic.get_velocity_x() + (m_side_dir == Direction::LEFT ? -10.f : 10.f)), 0.f);
    }
    break;
  case RECOVERING:
    if (returned_down || returned_up || returned_left || returned_right)
    {
      set_pos(Vector(m_sideways ? m_start_position.x : get_pos().x,
        m_sideways ? get_pos().y : m_start_position.y));
      m_physic.set_velocity(0.f, 0.f);
      if (m_ic_size == LARGE)
        m_cooldown_timer = PAUSE_TIME_LARGE;
      else
        m_cooldown_timer = PAUSE_TIME_NORMAL;
      set_state(IDLE);
    }
    else
    {
      m_physic.set_velocity(Vector(recover_x, recover_y)*(m_ic_size == LARGE ? 1.f : 1.125f));
    }
    break;
  default:
    log_debug << "IceCrusher in invalid state" << std::endl;
    break;
  }
}

void
IceCrusher::spawn_roots(Direction direction)
{
  if (m_sprite_name.find("root_crusher") == std::string::npos)
    return;

  Vector origin;
  Rectf test_solid_offset_1, test_solid_offset_2, test_empty_offset;
  bool vertical = false;

  switch (direction)
  {
  case Direction::DOWN:
    vertical = true;
    origin.x = m_col.m_bbox.get_middle().x - 16.f;
    origin.y = m_col.m_bbox.get_bottom();
    test_empty_offset = Rectf(Vector(4, -4), Size(16, 1));
    test_solid_offset_1 = Rectf(Vector(6, 8), Size(1, 1));
    test_solid_offset_2 = Rectf(Vector(16, 8), Size(1, 1));
    break;

  case Direction::LEFT:
    origin.x = m_col.m_bbox.get_left() - 6.f;
    origin.y = m_col.m_bbox.get_middle().y - 16.f;
    test_empty_offset = Rectf(Vector(8, 0), Size(1, 16));
    test_solid_offset_1 = Rectf(Vector(0, 4), Size(1, 1));
    test_solid_offset_2 = Rectf(Vector(0, 12), Size(1, 1));
    break;

  case Direction::RIGHT:
    origin.x = m_col.m_bbox.get_right() + 12.f;
    origin.y = m_col.m_bbox.get_middle().y - 16.f;
    test_empty_offset = Rectf(Vector(-16, 0), Size(1, 16));
    test_solid_offset_1 = Rectf(Vector(0, 4), Size(1, 1));
    test_solid_offset_2 = Rectf(Vector(0, 12), Size(1, 1));
    break;
  }

  for (float dir = -1.f; dir <= 1.f; dir += 2.f)
  {
    for (float step = 0.f; step < 3.f; step++)
    {
      Vector pos = origin;
      float dist = 32.f * step - 15.f;
      (vertical ? pos.x : pos.y) += dir * (dist + (vertical ? m_col.m_bbox.get_width() : m_col.m_bbox.get_height()));

      bool solid_1 = Sector::current()->is_free_of_tiles(test_solid_offset_1.moved(pos));
      bool solid_2 = Sector::current()->is_free_of_tiles(test_solid_offset_2.moved(pos));
      bool empty = Sector::current()->is_free_of_tiles(test_empty_offset.moved(pos));

      printf("Empty %d, solid1 %d, solid2 %d\n", empty, solid_1, solid_2);
      if (!empty || solid_1 || solid_2)
        break;

      Sector::current()->add<CrusherRoot>(pos, direction, step * .1f, m_layer);
    }
  }
}

void
IceCrusher::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), get_pos(), m_layer + 2, m_flip);
  if (m_sprite->has_action("whites"))
  {
    // draw icecrusher's eyes slightly behind
    m_lefteye->draw(context.color(), get_pos() + eye_position(false), m_layer + 1, m_flip);
    m_righteye->draw(context.color(), get_pos() + eye_position(true), m_layer + 1, m_flip);
    // draw the whites of icecrusher's eyes even further behind
    m_whites->draw(context.color(), get_pos(), m_layer, m_flip);
  }
}

void
IceCrusher::after_editor_set()
{
  MovingSprite::after_editor_set();
  after_sprite_set();
}

ObjectSettings
IceCrusher::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();
  result.add_bool(_("Sideways"), &m_sideways, "sideways", false);
  result.reorder({ "sideways", "sprite", "x", "y" });

  return result;
}

bool
IceCrusher::found_victim() const
{
  for (auto* player : Sector::get().get_players())
  {
    const Rectf& player_bbox = player->get_bbox();

    Rectf crush_area = get_bbox().grown(-1.f);
    if (!m_sideways)
    {
      crush_area.set_bottom(m_flip == NO_FLIP ? player_bbox.get_top() - 1.f : get_bbox().get_bottom() - 1.f);
      crush_area.set_top(m_flip != NO_FLIP ? player_bbox.get_bottom() + 1.f : get_bbox().get_top() + 1.f);
      if ((m_flip == NO_FLIP && player_bbox.get_top() >= get_bbox().get_bottom()) ||
        (m_flip != NO_FLIP && player_bbox.get_bottom() <= get_bbox().get_top()))
      {
        if ((player_bbox.get_right() > (get_bbox().get_left() - DROP_ACTIVATION_DISTANCE))
          && (player_bbox.get_left() < (get_bbox().get_right() + DROP_ACTIVATION_DISTANCE))
          && (Sector::get().is_free_of_statics(crush_area, this, false)) /* and area to player is free of objects */) {
          return true;
        }
      }
    }
    else
    {
      if (m_side_dir == Direction::LEFT)
      {
        crush_area.set_left(player_bbox.get_right() + 1);
        crush_area.set_right(get_bbox().get_right());
        if (((player_bbox.get_left()) <= get_bbox().get_left())
          && (player_bbox.get_bottom() + 5 > (get_bbox().get_top() - DROP_ACTIVATION_DISTANCE))
          && (player_bbox.get_top() < (get_bbox().get_bottom() + DROP_ACTIVATION_DISTANCE))
          && (Sector::get().is_free_of_statics(crush_area, this, false))		/* and area to player is free of objects */) {
          return true;
        }
      }
      else if (m_side_dir == Direction::RIGHT)
      {
        crush_area.set_right(player_bbox.get_left() - 1);
        crush_area.set_left(get_bbox().get_left());
        if (((player_bbox.get_right()) >= get_bbox().get_right())
          && (player_bbox.get_bottom() + 5 > (get_bbox().get_top() - DROP_ACTIVATION_DISTANCE))
          && (player_bbox.get_top() < (get_bbox().get_bottom() + DROP_ACTIVATION_DISTANCE))
          && (Sector::get().is_free_of_statics(crush_area, this, false))		/* and area to player is free of objects */) {
          return true;
        }
      }
    }
  }
  return false;
}

bool
IceCrusher::not_ice() const
{
  return (m_sprite_name.find("rock_crusher") != std::string::npos ||
    m_sprite_name.find("moss_crusher") != std::string::npos ||
    m_sprite_name.find("root_crusher") != std::string::npos);
}

void
IceCrusher::set_state(IceCrusherState state_, bool force)
{
  if ((m_state == state_) && (!force)) return;
  switch (state_)
  {
  case IDLE:
    m_sprite->set_action("idle");
    break;
  case CRUSHING:
    m_physic.reset();
    if (not_ice())
      m_sprite->set_action("crushing");
    break;
  case RECOVERING:
    if (not_ice())
      m_sprite->set_action("recovering");
    break;
  default:
    log_debug << "IceCrusher in invalid state" << std::endl;
    break;
  }
  m_physic.enable_gravity(false);
  m_state = state_;
}

void
IceCrusher::after_sprite_set()
{
  float sprite_width = static_cast<float>(m_sprite->get_width());
  float sprite_height = static_cast<float>(m_sprite->get_height());
  m_ic_size = (sprite_width*sprite_height >= 10000.f) ? LARGE : NORMAL;

  if (!m_sprite->has_action("whites"))
  {
    m_lefteye.reset();
    m_righteye.reset();
    m_whites.reset();
  }
  else
  {
    m_lefteye = m_sprite->clone();
    m_lefteye->set_action("lefteye");
    m_righteye = m_sprite->clone();
    m_righteye->set_action("righteye");
    m_whites = m_sprite->clone();
    m_whites->set_action("whites");
  }
}

Vector
IceCrusher::eye_position(bool right) const
{
  switch (m_state)
  {
  case IDLE:
    if (auto* player = Sector::get().get_nearest_player(m_col.m_bbox))
    {
      // Icecrusher focuses on approximate position of player's head
      const float player_focus_x = (player->get_bbox().get_right() + player->get_bbox().get_left()) * 0.5f;
      const float player_focus_y = player->get_bbox().get_bottom() * 0.25f + player->get_bbox().get_top() * 0.75f;
      // Icecrusher's approximate origin of line-of-sight
      const float crusher_origin_x = get_bbox().get_middle().x;
      const float crusher_origin_y = get_bbox().get_middle().y;
      // Line-of-sight displacement from icecrusher to player
      const float displacement_x = player_focus_x - crusher_origin_x;
      const float displacement_y = player_focus_y - crusher_origin_y;
      const float displacement_mag = powf(powf(displacement_x, 2.0f) + powf(displacement_y, 2.0f), 0.5f);
      // Determine weighting for eye displacement along x given icecrusher eye shape
      int weight_x = m_sprite->get_width() / 64 * (((displacement_x > 0) == right) ? 1 : 4);
      int weight_y = m_sprite->get_width() / 64 * 2;

      return Vector(displacement_x / displacement_mag * static_cast<float>(weight_x),
        displacement_y / displacement_mag * static_cast<float>(weight_y) - static_cast<float>(weight_y));
    }
    break;
  case CRUSHING:
    if (auto* player = Sector::get().get_nearest_player(m_col.m_bbox))
    {
      const float displacement_x = m_side_dir == Direction::LEFT ? -1.f : 1.f;
      int weight_x = m_sprite->get_width() / 64 * (((displacement_x > 0) == right) ? 1 : 4);
      int weight_y = m_sprite->get_width() / 64 * 2;

      return Vector(m_sideways ? static_cast<float>(weight_x) * (m_side_dir == Direction::LEFT ? -1 : 1.f) : 0.f,
        m_sideways ? -static_cast<float>(weight_y) : 0.f);
    }
    break;
  case RECOVERING:
    // Eyes spin while icecrusher is recovering, giving a dazed impression
    return Vector(sinf((right ? 1 : -1) * // X motion of each eye is opposite of the other
      ((!m_sideways ? get_pos().y / 13 : get_pos().x / 13) - // Phase factor due to y position
      (m_ic_size == NORMAL ? RECOVER_SPEED_NORMAL : RECOVER_SPEED_LARGE) + m_cooldown_timer * 13.0f)) * //Phase factor due to cooldown timer
      static_cast<float>(m_sprite->get_width()) / 64.0f * 2.0f - (right ? 1 : -1) * // Amplitude dependent on size
      static_cast<float>(m_sprite->get_width()) / 64.0f * 2.0f, // Offset to keep eyes visible

      cosf((right ? 3.1415f : 0.0f) + // Eyes spin out of phase of eachother
      (!m_sideways ? get_pos().y / 13 : get_pos().x / 13) - // Phase factor due to y position
        (m_ic_size == NORMAL ? RECOVER_SPEED_NORMAL : RECOVER_SPEED_LARGE) + m_cooldown_timer * 13.0f) * //Phase factor due to cooldown timer
      static_cast<float>(m_sprite->get_width()) / 64.0f * 2.0f -  // Amplitude dependent on size
      static_cast<float>(m_sprite->get_width()) / 64.0f * 2.0f); // Offset to keep eyes visible
    //no break because compiler throws errors if it does
    //break;
  default:
    log_debug << "IceCrusher in invalid state" << std::endl;
    break;
  }
  return Vector(0, 0);
}

void
IceCrusher::on_flip(float height)
{
  MovingSprite::on_flip(height);
  m_start_position.y = height - m_col.m_bbox.get_height() - m_start_position.y;
  FlipLevelTransformer::transform_flip(m_flip);
}

CrusherRoot::CrusherRoot(Vector position, IceCrusher::Direction direction, float delay, int layer) :
  MovingSprite(position, direction == IceCrusher::Direction::DOWN ?
    "images/creatures/icecrusher/roots/crusher_root.sprite" :
    "images/creatures/icecrusher/roots/crusher_root_side.sprite"),
  m_original_pos(position),
  m_direction(direction),
  m_delay_remaining(delay)
{
  m_layer = layer;

  if (delay_gone())
  {
    start_animation();
  }
  else
  {
    m_col.m_group = COLGROUP_DISABLED;
  }
}

HitResponse
CrusherRoot::collision(GameObject& other, const CollisionHit& hit)
{
  if (delay_gone())
  {
    auto player = dynamic_cast<Player*>(&other);

    if (player)
      player->kill(false);
  }

  return ABORT_MOVE;
}

void
CrusherRoot::update(float dt_sec)
{
  if (m_delay_remaining > 0.f)
  {
    m_delay_remaining -= dt_sec;

    if (delay_gone())
    {
      start_animation();
    }
    else
    {
      return;
    }
  }
  else if (m_sprite->animation_done())
  {
    remove_me();
    return;
  }

  switch (m_direction)
  {
  case IceCrusher::Direction::DOWN:
    m_col.move_to(m_original_pos + Vector(0, -m_sprite->get_current_hitbox_height()));
    break;

  case IceCrusher::Direction::LEFT:
    m_col.move_to(m_original_pos);
    break;

  case IceCrusher::Direction::RIGHT:
    m_col.move_to(m_original_pos + Vector(-m_sprite->get_current_hitbox_width(), 0));
    break;
  }
}

void
CrusherRoot::start_animation()
{
  m_col.m_group = COLGROUP_TOUCHABLE;

  switch (m_direction)
  {
  case IceCrusher::Direction::DOWN:
    m_sprite->set_action("downwards");
    m_sprite->set_animation_loops(1);
    break;

  case IceCrusher::Direction::LEFT:
    m_sprite->set_action("sideways-left");
    m_sprite->set_animation_loops(1);
    break;

  case IceCrusher::Direction::RIGHT:
    m_sprite->set_action("sideways-right");
    m_sprite->set_animation_loops(1);
    break;
  }
}

/* EOF */
