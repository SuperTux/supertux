//  SuperTux
//  Copyright (C) 2022 Daniel Ward <weluvgoatz@gmail.com>
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

#include "object/key.hpp"

#include "object/player.hpp"
#include "supertux/sector.hpp"
#include "trigger/door.hpp"
#include "util/reader_mapping.hpp"

Key::Key(const ReaderMapping& mapping) :
  MovingSprite(mapping, "images/objects/keys/key.sprite", LAYER_OBJECTS, COLGROUP_TOUCHABLE),
  m_pos_list(),
  m_collected(),
  m_state(KeyState::NORMAL),
  m_wait_timer(),
  m_unlock_timer(),
  m_chain_pos(1),
  m_my_door_pos(0.f, 0.f),
  m_color(Color::WHITE)
{
  std::vector<float> vColor;
  if (mapping.get("color", vColor)) {
    m_color = Color(vColor);
  }
  else
  {
      m_color = Color::WHITE;
  }

  m_sprite->set_color(m_color);
  m_physic.enable_gravity(false);
}

void
Key::update(float dt_sec)
{
  auto player = Sector::get().get_nearest_player(get_pos());
  if (!player)
    return;

  float distance = glm::length(get_pos() - player->get_pos());
  Vector chase_dir = glm::normalize(player->get_pos() - get_pos());

  if (m_state != KeyState::NORMAL)
  {
    m_pos_list.push_front((player->get_bbox().get_middle())-
      Vector(get_bbox().get_width()/2.f, get_bbox().get_height()/2.f));
  }
  Vector m_goal_pos = m_pos_list.back();

  while (m_pos_list.size() > unsigned(40 * m_chain_pos))
    m_pos_list.pop_back();

  // use
  for (auto& door : Sector::get().get_objects_by_type<Door>()) {
    if (door.is_locked() && glm::length(get_pos() - door.get_pos()) < 100.f) {
      player->add_collected_keys(-1);
      for (auto& key : Sector::get().get_objects_by_type<Key>()) {
        key.m_chain_pos -= 1;
      }
      door.unlock();
      m_my_door_pos = door.get_bbox().get_middle();
      m_unlock_timer.start(1.f);
      if (m_state != KeyState::FOUND) m_state = KeyState::FOUND;
    }
  }

  switch (m_state) {
  case NORMAL:
    break;
  case COLLECT:
    if (m_wait_timer.check())
    {
      m_wait_timer.stop();
      m_state = KeyState::FOLLOW;
    }
    break;
  case FOLLOW:
    m_col.set_movement((m_goal_pos - get_pos()) * (std::max(0.f, distance-(50.f*float(m_chain_pos)))/450.f));
    break;
  case FOUND:
    m_col.set_movement((m_my_door_pos - Vector(get_bbox().get_width() / 2.f, get_bbox().get_height() / 2.f) -
      (get_pos()))
      *glm::length((m_my_door_pos - get_bbox().get_middle())*0.003f));
    if (m_unlock_timer.check())
    {
      m_unlock_timer.stop();
      m_physic.enable_gravity(true);
      m_physic.set_velocity_y(-300.f);
      m_physic.set_acceleration_y(500.f);
      m_state = KeyState::USE;
    }
    break;
  case USE:
    m_col.set_movement(m_physic.get_movement(dt_sec));
    if (get_pos().y > Sector::get().get_height())
      remove_me();
    break;
  }

  GameObject::update(dt_sec);
}

HitResponse
Key::collision(GameObject& other, const CollisionHit& hit_)
{
  auto player = dynamic_cast<Player*> (&other);
  if (player && m_state == KeyState::NORMAL)
  {
    m_chain_pos = player->get_collected_keys() + 1;
    player->add_collected_keys(1);
    m_collected = true;
    m_wait_timer.start(0.5f);
    m_state = KeyState::COLLECT;
  }
  return FORCE_MOVE;
}

ObjectSettings
Key::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.add_color(_("Color"), &m_color, "color", Color::WHITE);

  result.reorder({ "color", "name", "x", "y" });

  return result;
}

/* EOF */
