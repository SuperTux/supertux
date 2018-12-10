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

#include <physfs.h>

#include "object/block.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "badguy/bomb.hpp"
#include "math/random.hpp"
#include "object/coin.hpp"
#include "object/growup.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/constants.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

static const float BOUNCY_BRICK_MAX_OFFSET = 8;
static const float BOUNCY_BRICK_SPEED = 90;
static const float BUMP_ROTATION_ANGLE = 10;

Block::Block(SpritePtr newsprite) :
  sprite(std::move(newsprite)),
  sprite_name(),
  default_sprite_name(),
  bouncing(false),
  breaking(false),
  bounce_dir(0),
  bounce_offset(0),
  original_y(-1)
{
  m_col.m_bbox.set_size(32, 32.1f);
  set_group(COLGROUP_STATIC);
  SoundManager::current()->preload("sounds/upgrade.wav");
  SoundManager::current()->preload("sounds/brick.wav");
}

Block::Block(const ReaderMapping& mapping, const std::string& sprite_file) :
  sprite(),
  sprite_name(),
  default_sprite_name(),
  bouncing(false),
  breaking(false),
  bounce_dir(0),
  bounce_offset(0),
  original_y(-1)
{
  mapping.get("x", m_col.m_bbox.get_left());
  mapping.get("y", m_col.m_bbox.get_top());

  std::string sf;
  mapping.get("sprite", sf);
  if (sf.empty() || !PHYSFS_exists(sf.c_str())) {
    sf = sprite_file;
  }
  sprite = SpriteManager::current()->create(sf);
  sprite_name = sf;
  default_sprite_name = sprite_name;

  m_col.m_bbox.set_size(32, 32.1f);
  set_group(COLGROUP_STATIC);
  SoundManager::current()->preload("sounds/upgrade.wav");
  SoundManager::current()->preload("sounds/brick.wav");
}

HitResponse
Block::collision(GameObject& other, const CollisionHit& )
{
  auto player = dynamic_cast<Player*> (&other);
  if (player) {
    if (player->get_bbox().get_top() > m_col.m_bbox.get_bottom() - SHIFT_DELTA) {
      hit(*player);
    }
  }

  // only interact with other objects if...
  //   1) we are bouncing
  //   2) the object is not portable (either never or not currently)
  //   3) the object is being hit from below (baguys don't get killed for activating boxes)
  auto portable = dynamic_cast<Portable*> (&other);
  auto moving_object = dynamic_cast<MovingObject*> (&other);
  auto bomb = dynamic_cast<Bomb*> (&other);
  bool is_portable = ((portable != nullptr) && portable->is_portable());
  bool is_bomb = (bomb != nullptr); // bombs need to explode, although they are considered portable
  bool hit_mo_from_below = ((moving_object == nullptr) || (moving_object->get_bbox().get_bottom() < (m_col.m_bbox.get_top() + SHIFT_DELTA)));
  if (bouncing && (!is_portable || is_bomb) && hit_mo_from_below) {

    // Badguys get killed
    auto badguy = dynamic_cast<BadGuy*> (&other);
    if (badguy) {
      badguy->kill_fall();
    }

    // Coins get collected
    auto coin = dynamic_cast<Coin*> (&other);
    if (coin) {
      coin->collect();
    }

    //Eggs get jumped
    auto growup = dynamic_cast<GrowUp*> (&other);
    if (growup) {
      growup->do_jump();
    }

  }

  return FORCE_MOVE;
}

void
Block::update(float dt_sec)
{
  if (!bouncing)
    return;

  float offset = original_y - get_pos().y;
  if (offset > BOUNCY_BRICK_MAX_OFFSET) {
    bounce_dir = BOUNCY_BRICK_SPEED;
    m_col.m_movement = Vector(0, bounce_dir * dt_sec);
    if (breaking){
      break_me();
    }
  } else if (offset < BOUNCY_BRICK_SPEED * dt_sec && bounce_dir > 0) {
    m_col.m_movement = Vector(0, offset);
    bounce_dir = 0;
    bouncing = false;
    sprite->set_angle(0);
  } else {
    m_col.m_movement = Vector(0, bounce_dir * dt_sec);
  }
}

void
Block::draw(DrawingContext& context)
{
  sprite->draw(context.color(), get_pos(), LAYER_OBJECTS+1);
}

void
Block::start_bounce(GameObject* hitter)
{
  if (original_y == -1){
    original_y = m_col.m_bbox.get_top();
  }
  bouncing = true;
  bounce_dir = -BOUNCY_BRICK_SPEED;
  bounce_offset = 0;

  MovingObject* hitter_mo = dynamic_cast<MovingObject*>(hitter);
  if (hitter_mo) {
    float center_of_hitter = hitter_mo->get_bbox().get_middle().x;
    float offset = (m_col.m_bbox.get_middle().x - center_of_hitter)*2 / m_col.m_bbox.get_width();
    sprite->set_angle(BUMP_ROTATION_ANGLE*offset);
  }
}

void
Block::start_break(GameObject* hitter)
{
  start_bounce(hitter);
  breaking = true;
}

void
Block::break_me()
{
  const auto gravity = Sector::get().get_gravity() * 100;
  Vector pos = get_pos() + Vector(16.0f, 16.0f);

  for (const char* action : {"piece1", "piece2", "piece3", "piece4", "piece5", "piece6"})
  {
    Vector velocity(graphicsRandom.randf(-100, 100),
                    graphicsRandom.randf(-400, -300));
    Sector::get().add<SpriteParticle>(sprite->clone(), action,
                                pos, ANCHOR_MIDDLE,
                                velocity, Vector(0, gravity),
                                LAYER_OBJECTS + 1);
  }

  remove_me();
}

ObjectSettings
Block::get_settings()
{
  ObjectSettings result = MovingObject::get_settings();

  result.add_sprite(_("Sprite"), &sprite_name, "sprite", default_sprite_name);

  return result;
}

void Block::after_editor_set()
{
  sprite = SpriteManager::current()->create(sprite_name);
}

/* EOF */
