//  SuperTux - PushButton running a script
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

#include "object/pushbutton.hpp"

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "object/rock.hpp"
#include "sprite/sprite.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

namespace {
const std::string BUTTON_SOUND = "sounds/switch.ogg";
//14 -> 8
}

PushButton::PushButton(const ReaderMapping& mapping) :
  MovingSprite(mapping, "images/objects/pushbutton/pushbutton.sprite", LAYER_BACKGROUNDTILES+1, COLGROUP_MOVING),
  script(),
  state(OFF),
  m_upside_down(false)
{
  SoundManager::current()->preload(BUTTON_SOUND);
  set_action("off", -1);
  m_col.m_bbox.set_size(m_sprite->get_current_hitbox_width(), m_sprite->get_current_hitbox_height());

  if (!mapping.get("script", script))
  {
    log_warning << "No script set for pushbutton." << std::endl;
  }

  mapping.get("upside-down", m_upside_down);
  if (m_upside_down)
    FlipLevelTransformer::transform_flip(m_flip);
}

ObjectSettings
PushButton::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.add_script(_("Script"), &script, "script");
  result.add_bool(_("Upside down"), &m_upside_down, "upside-down");

  result.reorder({"script", "upside-down", "x", "y"});

  return result;
}

void
PushButton::after_editor_set()
{
  MovingSprite::after_editor_set();
  if ((m_upside_down && m_flip == NO_FLIP) || (!m_upside_down && m_flip == VERTICAL_FLIP))
    FlipLevelTransformer::transform_flip(m_flip);
}

void
PushButton::update(float /*dt_sec*/)
{
}

HitResponse
PushButton::collision(GameObject& other, const CollisionHit& hit)
{
  auto player = dynamic_cast<Player*>(&other);
  auto rock = dynamic_cast<Rock*>(&other);
  if (!player && !rock)
    return FORCE_MOVE;
	if (player)
  {
    float vy = player->get_physic().get_velocity_y();

    if (m_upside_down)
    {
      if (vy >= 0)
        return FORCE_MOVE;

      if (hit.bottom)
        player->get_physic().set_velocity_y(0);
    }
    else
    {
      if (vy <= 0)
        return FORCE_MOVE;

      if (hit.top)
      {
        player->get_physic().set_velocity_y(0);
        player->set_on_ground(true);
      }
    }
	}

  if (state != OFF || !(m_upside_down ? hit.bottom : hit.top))
    return FORCE_MOVE;

  // change appearance
  state = ON;
  float old_bbox_height = m_col.m_bbox.get_height();
  set_action("on", -1);
  float new_bbox_height = m_col.m_bbox.get_height();
  Vector delta(0, old_bbox_height - new_bbox_height);
  set_pos(get_pos() + delta * (m_upside_down ? 0 : 1.f));

  // play sound
  SoundManager::current()->play(BUTTON_SOUND, get_pos());

  // run script
  Sector::get().run_script(script, "PushButton");

  return FORCE_MOVE;
}

void
PushButton::on_flip(float height)
{
  MovingSprite::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
  m_upside_down = !m_upside_down;
}

/* EOF */
