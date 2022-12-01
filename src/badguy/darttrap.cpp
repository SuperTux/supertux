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

namespace {
const float MUZZLE_Y = 25; /**< [px] muzzle y-offset from top */
}

DartTrap::DartTrap(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/darttrap/darttrap.sprite", LAYER_TILES-1),
  enabled(true),
  initial_delay(),
  fire_delay(),
  ammo(),
  state(IDLE),
  fire_timer()
{
  reader.get("enabled", enabled, true);
  reader.get("initial-delay", initial_delay, 0.0f);
  reader.get("fire-delay", fire_delay, 2.0f);
  reader.get("ammo", ammo, -1);
  m_countMe = false;
  SoundManager::current()->preload("sounds/dartfire.wav");
  if (m_start_dir == Direction::AUTO) { log_warning << "Setting a DartTrap's direction to AUTO is no good idea" << std::endl; }
  state = IDLE;
  set_colgroup_active(COLGROUP_DISABLED);

  if (!Editor::is_active()) {
    if (initial_delay == 0) initial_delay = 0.1f;
  }
}

void
DartTrap::initialize()
{
  m_sprite->set_action("idle", m_dir);
}

void
DartTrap::activate()
{
  fire_timer.start(initial_delay);
}

HitResponse
DartTrap::collision_player(Player& , const CollisionHit& )
{
  return ABORT_MOVE;
}

void
DartTrap::active_update(float )
{
  if (!enabled) {
    return;
  }
  switch (state) {
    case IDLE:
      if ((ammo != 0) && (fire_timer.check())) {
        if (ammo > 0) ammo--;
        load();
        fire_timer.start(fire_delay);
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
  state = LOADING;
  m_sprite->set_action("loading", m_dir, 1);
}

void
DartTrap::fire()
{
  float px = get_pos().x;
  if (m_dir == Direction::RIGHT) px += 5;
  float py = get_pos().y;
  if (m_flip == NO_FLIP)
    py += MUZZLE_Y;
  else
    py += (m_col.m_bbox.get_height() - MUZZLE_Y - 7.0f);

  SoundManager::current()->play("sounds/dartfire.wav", get_pos());
  Sector::get().add<Dart>(Vector(px, py), m_dir, this);
  state = IDLE;
  m_sprite->set_action("idle", m_dir);
}

ObjectSettings
DartTrap::get_settings()
{
  ObjectSettings result = BadGuy::get_settings();

  result.add_float(_("Initial delay"), &initial_delay, "initial-delay");
  result.add_bool(_("Enabled"), &enabled, "enabled", true);
  result.add_float(_("Fire delay"), &fire_delay, "fire-delay");
  result.add_int(_("Ammo"), &ammo, "ammo");

  result.reorder({"initial-delay", "fire-delay", "ammo", "direction", "x", "y"});

  return result;
}

void
DartTrap::on_flip(float height)
{
  BadGuy::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
