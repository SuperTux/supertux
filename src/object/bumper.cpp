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
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

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
    SoundManager::current()->play(TRAMPOLINE_SOUND);
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

BEGIN_BACKUP(Bumper, MovingSprite);
  SAVE_OBJECT(physic);
  SAVE_PRIMITIVE(left); // TODO: Shouldn't be needed, bumpers aren't created or
                        // destroyed dynamically and left/right doesn't change
                        // during playtime
END_BACKUP(Bumper);

BEGIN_RESTORE_WITH_SUBREADER(Bumper, MovingSprite);
  LOAD_OBJECT(physic);
  LOAD_PRIMITIVE(left);
END_RESTORE(Bumper);

/* EOF */
