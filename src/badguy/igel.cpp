//  SuperTux - Badguy "Igel"
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

#include "badguy/igel.hpp"

#include <variant>

#include "collision/collision_system.hpp"
#include "object/player.hpp"
#include "supertux/sector.hpp"

namespace {

const float IGEL_SPEED = 80; /**< Speed at which we walk around. */
const float TURN_RECOVER_TIME = 0.5; /**< Seconds before we will again turn around when shot at. */
const float RANGE_OF_VISION = 256; /**< Sange in px at which we can see bullets. */

} // namespace

Igel::Igel(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/igel/igel.sprite", "left", "right"),
  m_state(STATE_NORMAL),
  m_roll_timer()
{
  walk_speed = IGEL_SPEED;
  max_drop_height = 16;
}

void
Igel::active_update(float dt_sec)
{
  WalkingBadguy::active_update(dt_sec);
  switch (m_state)
  {
    case STATE_ROLLING:
      if (get_action() == "roll-start-"+dir_to_string(m_dir) &&
          m_sprite->animation_done())
      {
        set_action("roll", m_dir);
      }

      if (m_roll_timer.check())
      {
        stop_rolling();
        break;
      }

      break;

    case STATE_NORMAL:
      if (get_action() == "roll-end-"+dir_to_string(m_dir) &&
          m_sprite->animation_done())
      {
        set_action(m_dir);
      }
      try_roll();
      break;
  }
}

GameObjectTypes Igel::get_types() const
{
  return {
    { "normal", _("Normal") },
    { "corrupted", _("Corrupted") }
  };
}

std::string Igel::get_default_sprite_name() const
{
  switch (m_type)
  {
    case NORMAL: return "images/creatures/igel/igel.sprite";
    case CORRUPTED: return "images/creatures/igel/corrupted/corrupted_igel.sprite";
  }
  return "images/creatures/igel/igel.sprite";
}

bool Igel::try_roll()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  Player* plr = get_nearest_player();
  if (!plr) return false;

  Vector mid = get_bbox().get_middle();
  Vector range = {mid.x + (32*10 * (m_dir == Direction::LEFT ? -1 : 1)), mid.y};

  RaycastResult result = Sector::get().get_first_line_intersection(mid, range, false, get_collision_object());

  auto* obj = std::get_if<CollisionObject*>(&result.hit);
  if (!obj || *obj != plr->get_collision_object()) return false;

  roll();

  return true;
}

void Igel::roll()
{
  m_state = STATE_ROLLING;
  set_action("roll-start", m_dir);
  set_walk_speed(250);
  m_physic.set_velocity_x(250 * (m_dir == Direction::LEFT ? -1 : 1));
  max_drop_height = 600;
  m_roll_timer.start(2.f);
}

void Igel::stop_rolling()
{
  m_state = STATE_NORMAL;
  set_action("roll-end", m_dir);
  set_walk_speed(IGEL_SPEED);
  m_physic.set_velocity_x(IGEL_SPEED * (m_dir == Direction::LEFT ? -1 : 1));
  max_drop_height = 16;
}

/* EOF */
