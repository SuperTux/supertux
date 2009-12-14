//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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

#include "object/bonus_block.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "lisp/list_iterator.hpp"
#include "object/broken_brick.hpp"
#include "object/flower.hpp"
#include "object/bouncy_coin.hpp"
#include "object/growup.hpp"
#include "object/oneup.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "object/specialriser.hpp"
#include "object/star.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/constants.hpp"
#include "supertux/level.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

BonusBlock::BonusBlock(const Vector& pos, int data) :
  Block(sprite_manager->create("images/objects/bonus_block/bonusblock.sprite")), 
  contents(),
  object(0)
{
  bbox.set_pos(pos);
  sprite->set_action("normal");
  switch(data) {
    case 1: contents = CONTENT_COIN; break;
    case 2: contents = CONTENT_FIREGROW; break;
    case 3: contents = CONTENT_STAR; break;
    case 4: contents = CONTENT_1UP; break;
    case 5: contents = CONTENT_ICEGROW; break;
    default:
      log_warning << "Invalid box contents" << std::endl;
      contents = CONTENT_COIN;
      break;
  }
}

BonusBlock::BonusBlock(const Reader& lisp) :
  Block(sprite_manager->create("images/objects/bonus_block/bonusblock.sprite")),
  contents(),
  object(0)
{
  Vector pos;

  contents = CONTENT_COIN;
  lisp::ListIterator iter(&lisp);
  while(iter.next()) {
    const std::string& token = iter.item();
    if(token == "x") {
      iter.value()->get(pos.x);
    } else if(token == "y") {
      iter.value()->get(pos.y);
    } else if(token == "contents") {
      std::string contentstring;
      iter.value()->get(contentstring);
      if(contentstring == "coin") {
        contents = CONTENT_COIN;
      } else if(contentstring == "firegrow") {
        contents = CONTENT_FIREGROW;
      } else if(contentstring == "icegrow") {
        contents = CONTENT_ICEGROW;
      } else if(contentstring == "star") {
        contents = CONTENT_STAR;
      } else if(contentstring == "1up") {
        contents = CONTENT_1UP;
      } else if(contentstring == "custom") {
        contents = CONTENT_CUSTOM;
      } else {
        log_warning << "Invalid box contents '" << contentstring << "'" << std::endl;
      }
    } else {
      if(contents == CONTENT_CUSTOM) {
        GameObject* game_object = ObjectFactory::instance().create(token, *(iter.lisp()));
        object = dynamic_cast<MovingObject*> (game_object);
        if(object == 0)
          throw std::runtime_error(
            "Only MovingObjects are allowed inside BonusBlocks");
      } else {
        log_warning << "Invalid element '" << token << "' in bonusblock" << std::endl;
      }
    }
  }

  if(contents == CONTENT_CUSTOM && object == 0)
    throw std::runtime_error("Need to specify content object for custom block");

  bbox.set_pos(pos);
}

BonusBlock::~BonusBlock()
{
  delete object;
}

void
BonusBlock::hit(Player& )
{
  try_open();
}

HitResponse
BonusBlock::collision(GameObject& other, const CollisionHit& hit){

  Player* player = dynamic_cast<Player*> (&other);
  if (player) {
    if (player->does_buttjump) try_open();
  }

  BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
  if(badguy) {
    // hit contains no information for collisions with blocks.
    // Badguy's bottom has to be below the top of the block
    // SHIFT_DELTA is required to slide over one tile gaps.
    if( badguy->can_break() && ( badguy->get_bbox().get_bottom() > get_bbox().get_top() + SHIFT_DELTA ) ){
      try_open();
    }
  }
  Portable* portable = dynamic_cast<Portable*> (&other);
  if(portable) {
    MovingObject* moving = dynamic_cast<MovingObject*> (&other);
    if(moving->get_bbox().get_top() > get_bbox().get_bottom() - SHIFT_DELTA) {
      try_open();
    }
  }
  return Block::collision(other, hit);
}

void
BonusBlock::try_open()
{
  if(sprite->get_action() == "empty") {
    sound_manager->play("sounds/brick.wav");
    return;
  }

  Sector* sector = Sector::current();
  assert(sector);
  assert(sector->player);
  Player& player = *(sector->player);
  Direction direction = (player.get_bbox().get_middle().x > get_bbox().get_middle().x) ? LEFT : RIGHT;

  switch(contents) {
    case CONTENT_COIN:
      Sector::current()->add_object(new BouncyCoin(get_pos(), true));
      player.get_status()->add_coins(1);
      Sector::current()->get_level()->stats.coins++;
      break;

    case CONTENT_FIREGROW:
      if(player.get_status()->bonus == NO_BONUS) {
        SpecialRiser* riser = new SpecialRiser(get_pos(), new GrowUp(direction));
        sector->add_object(riser);
      } else {
        SpecialRiser* riser = new SpecialRiser(
          get_pos(), new Flower(FIRE_BONUS));
        sector->add_object(riser);
      }
      sound_manager->play("sounds/upgrade.wav");
      break;

    case CONTENT_ICEGROW:
      if(player.get_status()->bonus == NO_BONUS) {
        SpecialRiser* riser = new SpecialRiser(get_pos(), new GrowUp(direction));
        sector->add_object(riser);
      } else {
        SpecialRiser* riser = new SpecialRiser(
          get_pos(), new Flower(ICE_BONUS));
        sector->add_object(riser);
      }
      sound_manager->play("sounds/upgrade.wav");
      break;

    case CONTENT_STAR:
      sector->add_object(new Star(get_pos() + Vector(0, -32), direction));
      break;

    case CONTENT_1UP:
      sector->add_object(new OneUp(get_pos(), direction));
      break;

    case CONTENT_CUSTOM:
      SpecialRiser* riser = new SpecialRiser(get_pos(), object);
      object = 0;
      sector->add_object(riser);
      sound_manager->play("sounds/upgrade.wav");
      break;
  }

  start_bounce(&player);
  sprite->set_action("empty");
}

void
Block::break_me()
{
  Sector* sector = Sector::current();
  sector->add_object(
    new BrokenBrick(sprite->clone(), get_pos(), Vector(-100, -400)));
  sector->add_object(
    new BrokenBrick(sprite->clone(), get_pos() + Vector(0, 16),
                    Vector(-150, -300)));
  sector->add_object(
    new BrokenBrick(sprite->clone(), get_pos() + Vector(16, 0),
                    Vector(100, -400)));
  sector->add_object(
    new BrokenBrick(sprite->clone(), get_pos() + Vector(16, 16),
                    Vector(150, -300)));
  remove_me();
}

/* EOF */
