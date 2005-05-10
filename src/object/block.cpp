//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include <config.h>

#include "block.h"
#include "resources.h"
#include "player.h"
#include "sector.h"
#include "sprite/sprite.h"
#include "sprite/sprite_manager.h"
#include "video/drawing_context.h"
#include "lisp/lisp.h"
#include "gameobjs.h"
#include "specialriser.h"
#include "growup.h"
#include "flower.h"
#include "oneup.h"
#include "star.h"
#include "player_status.h"
#include "badguy/badguy.h"
#include "coin.h"
#include "object_factory.h"

static const float BOUNCY_BRICK_MAX_OFFSET=8;
static const float BOUNCY_BRICK_SPEED=90;
static const float EPSILON = .0001;

Block::Block(Sprite* newsprite)
  : sprite(newsprite), bouncing(false), bounce_dir(0), bounce_offset(0)
{
  bbox.set_size(32, 32.1);
  flags |= FLAG_SOLID;
}

Block::~Block()
{
  delete sprite;
}

HitResponse
Block::collision(GameObject& other, const CollisionHit& hitdata)
{
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    // collided from below?
    if(hitdata.normal.x == 0 && hitdata.normal.y < 0) {
      hit(*player);
    }
  }

  if(bouncing) {
    BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
    if(badguy) {
      badguy->kill_fall();
    }
    Coin* coin = dynamic_cast<Coin*> (&other);
    if(coin) {
      coin->collect();
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
  } else if(offset < BOUNCY_BRICK_SPEED * elapsed_time && bounce_dir > 0) {
    movement = Vector(0, offset);
    bounce_dir = 0;
    bouncing = false;
  } else {
    movement = Vector(0, bounce_dir * elapsed_time);
  }
}

void
Block::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS+1);
}

void
Block::start_bounce()
{
  original_y = bbox.p1.y;
  bouncing = true;
  bounce_dir = -BOUNCY_BRICK_SPEED;
  bounce_offset = 0;
}

//---------------------------------------------------------------------------

BonusBlock::BonusBlock(const Vector& pos, int data)
  : Block(sprite_manager->create("bonusblock"))
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
      std::cerr << "Invalid box contents!\n";
      contents = CONTENT_COIN;
      break;
  }          
}

BonusBlock::BonusBlock(const lisp::Lisp& lisp)
  : Block(sprite_manager->create("bonusblock"))
{
  Vector pos;
  lisp.get("x", pos.x);
  lisp.get("y", pos.y);
  bbox.set_pos(pos);

  std::string contentstring;
  contents = CONTENT_COIN;
  if(lisp.get("contents", contentstring)) {
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
    } else {
      std::cerr << "Invalid box contents '" << contentstring << "'.\n";
    }
  }
}

void
BonusBlock::hit(Player& )
{
  try_open();
}

void
BonusBlock::try_open()
{
  if(sprite->get_action_name() == "empty") {
    sound_manager->play_sound("brick");
    return;
  }
  
  Sector* sector = Sector::current();
  Player& player = *(sector->player);
  switch(contents) {
    case CONTENT_COIN:
      Sector::current()->add_object(new BouncyCoin(get_pos()));
      player.get_status()->incCoins();
      break;

    case CONTENT_FIREGROW:
      if(player.get_status()->bonus == NO_BONUS) {
        SpecialRiser* riser = new SpecialRiser(
            new GrowUp(get_pos() + Vector(0, -32)));
        sector->add_object(riser);
      } else {
        SpecialRiser* riser = new SpecialRiser(
            new Flower(get_pos() + Vector(0, -32), Flower::FIREFLOWER));
        sector->add_object(riser);
      }
      sound_manager->play_sound("upgrade");
      break;

    case CONTENT_ICEGROW:
      if(player.get_status()->bonus == NO_BONUS) {
        SpecialRiser* riser = new SpecialRiser(
            new GrowUp(get_pos() + Vector(0, -32)));
        sector->add_object(riser);                                            
      } else {
        SpecialRiser* riser = new SpecialRiser(                               
            new Flower(get_pos() + Vector(0, -32), Flower::ICEFLOWER));
        sector->add_object(riser);
      }      
      sound_manager->play_sound("upgrade");
      break;

    case CONTENT_STAR:
      sector->add_object(new Star(get_pos() + Vector(0, -32)));
      break;

    case CONTENT_1UP:
      sector->add_object(new OneUp(get_pos()));
      break;

    default:
      assert(false);
  }

  start_bounce();
  sprite->set_action("empty");
}

IMPLEMENT_FACTORY(BonusBlock, "bonusblock")

//---------------------------------------------------------------------------

Brick::Brick(const Vector& pos, int data)
  : Block(sprite_manager->create("brick")), breakable(false),
    coin_counter(0)
{
  bbox.set_pos(pos);
  if(data == 1)
    coin_counter = 5;
  else
    breakable = true;
}

void
Brick::hit(Player& )
{
  if(sprite->get_action_name() == "empty")
    return;
  
  try_break(true);
}

void
Brick::try_break(bool playerhit)
{
  if(sprite->get_action_name() == "empty")
    return;
  
  sound_manager->play_sound("brick");
  Sector* sector = Sector::current();
  Player& player = *(sector->player);
  if(coin_counter > 0) {
    sector->add_object(new BouncyCoin(get_pos()));
    coin_counter--;
    player.get_status()->incCoins();
    if(coin_counter == 0)
      sprite->set_action("empty");
    start_bounce();
  } else if(breakable) {
    if(playerhit && !player.is_big()) {
      start_bounce();
      return;
    }

    sector->add_object(
        new BrokenBrick(new Sprite(*sprite), get_pos(), Vector(-100, -400)));
    sector->add_object(
        new BrokenBrick(new Sprite(*sprite), get_pos() + Vector(0, 16),
          Vector(-150, -300)));
    sector->add_object(
        new BrokenBrick(new Sprite(*sprite), get_pos() + Vector(16, 0),
          Vector(100, -400)));
    sector->add_object(
        new BrokenBrick(new Sprite(*sprite), get_pos() + Vector(16, 16),
          Vector(150, -300)));
    remove_me();
  }
}

//IMPLEMENT_FACTORY(Brick, "brick")
