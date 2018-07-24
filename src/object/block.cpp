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
#include "object/broken_brick.hpp"
#include "object/coin.hpp"
#include "object/growup.hpp"
#include "object/player.hpp"
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
  sprite(newsprite),
  sprite_name(),
  default_sprite_name(),
  bouncing(false),
  breaking(false),
  bounce_dir(0),
  bounce_offset(0),
  original_y(-1)
{
  bbox.set_size(32, 32.1f);
  set_group(COLGROUP_STATIC);
  SoundManager::current()->preload("sounds/upgrade.wav");
  SoundManager::current()->preload("sounds/brick.wav");
}

Block::Block(const ReaderMapping& lisp, const std::string& sprite_file) :
  sprite(),
  sprite_name(),
  default_sprite_name(),
  bouncing(false),
  breaking(false),
  bounce_dir(0),
  bounce_offset(0),
  original_y(-1)
{
  lisp.get("x", bbox.p1.x);
  lisp.get("y", bbox.p1.y);

  std::string sf;
  lisp.get("sprite", sf);
  if (sf.empty() || !PHYSFS_exists(sf.c_str())) {
    sf = sprite_file;
  }
  sprite = SpriteManager::current()->create(sf);
  sprite_name = sf;
  default_sprite_name = sprite_name;

  bbox.set_size(32, 32.1f);
  set_group(COLGROUP_STATIC);
  SoundManager::current()->preload("sounds/upgrade.wav");
  SoundManager::current()->preload("sounds/brick.wav");
}

HitResponse
Block::collision(GameObject& other, const CollisionHit& )
{
  auto player = dynamic_cast<Player*> (&other);
  if(player) {
    if(player->get_bbox().get_top() > bbox.get_bottom() - SHIFT_DELTA) {
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
  bool is_portable = ((portable != 0) && portable->is_portable());
  bool is_bomb = (bomb != 0); // bombs need to explode, although they are considered portable
  bool hit_mo_from_below = ((moving_object == 0) || (moving_object->get_bbox().get_bottom() < (bbox.get_top() + SHIFT_DELTA)));
  if(bouncing && (!is_portable || is_bomb) && hit_mo_from_below) {

    // Badguys get killed
    auto badguy = dynamic_cast<BadGuy*> (&other);
    if(badguy) {
      badguy->kill_fall();
    }

    // Coins get collected
    auto coin = dynamic_cast<Coin*> (&other);
    if(coin) {
      coin->collect();
    }

    //Eggs get jumped
    auto growup = dynamic_cast<GrowUp*> (&other);
    if(growup) {
      growup->do_jump();
    }

  }

  return FORCE_MOVE;
}

void
Block::update(float elapsed_time)
{
  if(!bouncing)
    return;

  float offset = original_y - get_pos().y;
  if(offset > BOUNCY_BRICK_MAX_OFFSET) {
    bounce_dir = BOUNCY_BRICK_SPEED;
    movement = Vector(0, bounce_dir * elapsed_time);
    if(breaking){
      break_me();
    }
  } else if(offset < BOUNCY_BRICK_SPEED * elapsed_time && bounce_dir > 0) {
    movement = Vector(0, offset);
    bounce_dir = 0;
    bouncing = false;
    sprite->set_angle(0);
  } else {
    movement = Vector(0, bounce_dir * elapsed_time);
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
  if(original_y == -1){
    original_y = bbox.p1.y;
  }
  bouncing = true;
  bounce_dir = -BOUNCY_BRICK_SPEED;
  bounce_offset = 0;

  MovingObject* hitter_mo = dynamic_cast<MovingObject*>(hitter);
  if (hitter_mo) {
    float center_of_hitter = hitter_mo->get_bbox().get_middle().x;
    float offset = (bbox.get_middle().x - center_of_hitter)*2 / bbox.get_width();
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
  auto sector = Sector::current();
  sector->add_object(
    std::make_shared<BrokenBrick>(sprite->clone(), get_pos(), Vector(-100, -400)));
  sector->add_object(
    std::make_shared<BrokenBrick>(sprite->clone(), get_pos() + Vector(0, 16),
                                  Vector(-150, -300)));
  sector->add_object(
    std::make_shared<BrokenBrick>(sprite->clone(), get_pos() + Vector(16, 0),
                                  Vector(100, -400)));
  sector->add_object(
    std::make_shared<BrokenBrick>(sprite->clone(), get_pos() + Vector(16, 16),
                                  Vector(150, -300)));
  remove_me();
}

ObjectSettings Block::get_settings()
{
  ObjectSettings result = MovingObject::get_settings();
  ObjectOption spr(MN_FILE, _("Sprite"), &sprite_name);
  spr.select.push_back(".sprite");
  result.options.push_back(spr);
  return result;
}

void Block::after_editor_set()
{
  sprite = SpriteManager::current()->create(sprite_name);
}

void Block::save(Writer& writer)
{
  MovingObject::save(writer);
  if(sprite_name != get_default_sprite_name())
  {
    writer.write("sprite", sprite_name);
  }
}

/* EOF */
