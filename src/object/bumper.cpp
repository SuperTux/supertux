//  Copyright (C) 2020 Daniel Ward <weluvgoatz@gmail.com>
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

#include "object/bumper.hpp"

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "audio/sound_manager.hpp"
#include "object/fallblock.hpp"
#include "object/player.hpp"
#include "object/platform.hpp"
#include "object/rock.hpp"
#include "object/tilemap.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

namespace {
const std::string TRAMPOLINE_SOUND = "sounds/trampoline.wav";
const float BOUNCE_Y = -450.0f;
const float BOUNCE_X = 700.0f;
}

Bumper::Bumper(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/trampoline/bumper.sprite", LAYER_OBJECTS, COLGROUP_MOVING),
  physic(),
  left()
{
	reader.get("left", left);
  m_sprite->set_action(left ? "left-normal" : "right-normal");
	physic.enable_gravity(false);
}
  
ObjectSettings
Bumper::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.add_bool(_("Facing Left"), &left, "left", false);

  result.reorder({"left", "sprite", "x", "y"});

  return result;
}
  
void
Bumper::update(float dt_sec)
{
  if (m_sprite->animation_done())
  {
    m_sprite->set_action(left ? "left-normal" : "right-normal");
  }
  Rectf smallbox = get_bbox().grown(1.f);
  Rectf largebox = get_bbox().grown(8.f);

  for (auto& rock : Sector::get().get_objects_by_type<Rock>()) {
    if (smallbox.contains(rock.get_bbox()))
    {
      float BOUNCE_DIR = left ? -BOUNCE_X : BOUNCE_X;
      rock.get_physic().set_velocity(BOUNCE_DIR * 0.7f, BOUNCE_Y * 0.6f);
      SoundManager::current()->play(TRAMPOLINE_SOUND, get_pos());
      m_sprite->set_action((left ? "left-swinging" : "right-swinging"), 1);
    }
  }

  for (auto& tm : Sector::get().get_objects_by_type<TileMap>()) {
    if (largebox.contains(tm.get_bbox()) && tm.is_solid() && glm::length(tm.get_movement(true)) > (1.f*dt_sec))
    {
      m_col.set_movement(tm.get_movement(true));
      return;
    }
  }

  for (auto& platform : Sector::get().get_objects_by_type<Platform>()) {
    if (largebox.contains(platform.get_bbox()))
    {
      m_col.set_movement(platform.get_movement());
      return;
    }
  }

  for (auto& fallblock : Sector::get().get_objects_by_type<FallBlock>()) {
    if (smallbox.contains(fallblock.get_bbox()))
    {
      m_col.set_movement((fallblock.get_state() == FallBlock::State::LAND) ? Vector(0.f, 0.f) : fallblock.get_physic().get_movement(dt_sec));
      return;
    }
  }
  m_col.set_movement(physic.get_movement (dt_sec));
}

HitResponse
Bumper::collision(GameObject& other, const CollisionHit& hit)
{
  auto player = dynamic_cast<Player*> (&other);
  if (player)
  {
	  float BOUNCE_DIR = left ? -BOUNCE_X : BOUNCE_X;
	  player->get_physic().set_velocity(0.f, BOUNCE_Y);
    player->sideways_push(BOUNCE_DIR);
    SoundManager::current()->play(TRAMPOLINE_SOUND, get_pos());
    m_sprite->set_action((left ? "left-swinging" : "right-swinging"), 1);
  }
	
	auto bumper = dynamic_cast<Bumper*> (&other);
	if (bumper)
  {
    physic.set_velocity_y(0);
	  return FORCE_MOVE;
	}
  return ABORT_MOVE;
}

void
Bumper::on_flip(float height)
{
  MovingSprite::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
