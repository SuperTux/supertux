//  DartTrap - Shoots a Dart at regular intervals
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

#include "badguy/darttrap.hpp"

#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "badguy/dart.hpp"
#include "editor/editor.hpp"
#include "sprite/sprite.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

DartTrap::DartTrap(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/darttrap/darttrap.sprite", LAYER_TILES-1),
  m_enabled(true),
  m_initial_delay(),
  m_fire_delay(),
  m_ammo(),
  m_dart_sprite("images/creatures/dart/dart.sprite"),
  m_state(IDLE),
  m_fire_timer()
{
  reader.get("enabled", m_enabled, true);
  reader.get("initial-delay", m_initial_delay, 0.0f);
  reader.get("fire-delay", m_fire_delay, 2.0f);
  reader.get("ammo", m_ammo, -1);
  reader.get("dart-sprite", m_dart_sprite, "images/creatures/dart/dart.sprite");
  m_countMe = false;
  SoundManager::current()->preload("sounds/dartfire.wav");
  if (m_start_dir == Direction::AUTO) { log_warning << "Setting a DartTrap's direction to AUTO is no good idea" << std::endl; }
  m_state = IDLE;
  set_colgroup_active(COLGROUP_DISABLED);

  if (!Editor::is_active()) {
    if (m_initial_delay == 0) m_initial_delay = 0.1f;
  }
}

void
DartTrap::initialize()
{
  set_action("idle", m_dir == Direction::UP ? Direction::DOWN : m_dir);
  if (m_dir == Direction::UP) m_flip = VERTICAL_FLIP;
}

void
DartTrap::activate()
{
  m_fire_timer.start(m_initial_delay);
}

HitResponse
DartTrap::collision_player(Player&, const CollisionHit& )
{
  return ABORT_MOVE;
}

void
DartTrap::active_update(float)
{
  if (!m_enabled) return;

  switch (m_state) {
    case IDLE:
      if ((m_ammo != 0) && (m_fire_timer.check())) {
        if (m_ammo > 0) m_ammo--;
        load();
        m_fire_timer.start(m_fire_delay);
      }
      break;

    case LOADING:
      if (m_sprite->animation_done()) {
        fire();
      }
      break;

    default:
      break;
  }
}

void
DartTrap::load()
{
  m_state = LOADING;
  set_action("loading", m_dir == Direction::UP ? Direction::DOWN : m_dir, 1);
}

void
DartTrap::fire()
{
  SoundManager::current()->play("sounds/dartfire.wav", get_pos());
  Dart &dart = Sector::get().add<Dart>(Vector(0.f, 0.f), m_dir, this, m_dart_sprite, m_flip);

  Vector pos;
  switch (m_dir)
  {
    case Direction::RIGHT:
      pos = Vector(get_pos().x,
                   get_pos().y + m_col.m_bbox.get_height() / 2 - dart.get_bbox().get_height() / 2);
      break;
    case Direction::UP:
      pos = Vector(get_pos().x + m_col.m_bbox.get_width() / 2 - dart.get_bbox().get_width() / 2,
                   get_pos().y + m_col.m_bbox.get_height() - dart.get_bbox().get_height());
      break;
    case Direction::DOWN:
      pos = Vector(get_pos().x + m_col.m_bbox.get_width() / 2 - dart.get_bbox().get_width() / 2,
                   get_pos().y);
      break;
    default:
      pos = Vector(get_pos().x + m_col.m_bbox.get_width() - dart.get_bbox().get_width(),
                   get_pos().y + m_col.m_bbox.get_height() / 2 - dart.get_bbox().get_height() / 2);
      break;
  }

  dart.set_pos(pos);

  m_state = IDLE;
  set_action("idle", m_dir == Direction::UP ? Direction::DOWN : m_dir);
}

ObjectSettings
DartTrap::get_settings()
{
  ObjectSettings result = BadGuy::get_settings();

  result.add_float(_("Initial delay"), &m_initial_delay, "initial-delay");
  result.add_bool(_("Enabled"), &m_enabled, "enabled", true);
  result.add_float(_("Fire delay"), &m_fire_delay, "fire-delay");
  result.add_int(_("Ammo"), &m_ammo, "ammo");
  result.add_sprite(_("Dart sprite"), &m_dart_sprite, "dart-sprite", "images/creatures/dart/dart.sprite");

  result.reorder({"initial-delay", "fire-delay", "ammo", "direction", "x", "y", "dart-sprite"});

  return result;
}

void
DartTrap::after_editor_set()
{
  BadGuy::after_editor_set();
  if ((m_dir == Direction::UP && m_flip == NO_FLIP) || (m_dir == Direction::DOWN && m_flip == VERTICAL_FLIP))
    FlipLevelTransformer::transform_flip(m_flip);
}

void
DartTrap::on_flip(float height)
{
  BadGuy::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
  if (m_dir == Direction::UP)
    m_dir = Direction::DOWN;
  else if (m_dir == Direction::DOWN)
    m_dir = Direction::UP;
}

/* EOF */
