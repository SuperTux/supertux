#include <config.h>

#include "block.h"
#include "resources.h"
#include "player.h"
#include "sector.h"
#include "special/sprite.h"
#include "special/sprite_manager.h"
#include "video/drawing_context.h"
#include "gameobjs.h"
#include "specialriser.h"
#include "growup.h"
#include "flower.h"
#include "oneup.h"
#include "star.h"
#include "badguy/badguy.h"
#include "coin.h"
#include "object_factory.h"

static const float BOUNCY_BRICK_MAX_OFFSET=8;
static const float BOUNCY_BRICK_SPEED=90;
static const float EPSILON = .0001;

Block::Block(const Vector& pos, Sprite* newsprite)
  : sprite(newsprite), bouncing(false), bounce_dir(0), bounce_offset(0)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  flags |= FLAG_SOLID;
  original_y = pos.y;
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
Block::action(float elapsed_time)
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
  bouncing = true;
  bounce_dir = -BOUNCY_BRICK_SPEED;
  bounce_offset = 0;
}

//---------------------------------------------------------------------------

BonusBlock::BonusBlock(const Vector& pos, int newdata)
  : Block(pos, sprite_manager->create("bonusblock")), data(newdata)
{
  sprite->set_action("default");
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
    SoundManager::get()->play_sound(IDToSound(SND_BRICK));
    return;
  }
  
  Sector* sector = Sector::current();
  Player& player = *(sector->player);
  switch(data) {
    case 1: // coin
      Sector::current()->add_object(new BouncyCoin(get_pos()));
      player.get_status().incCoins();
      break;

    case 2: // grow/fireflower
      if(player.size == SMALL) {
        SpecialRiser* riser = new SpecialRiser(
            new GrowUp(get_pos() + Vector(0, -32)));
        sector->add_object(riser);
      } else {
        SpecialRiser* riser = new SpecialRiser(
            new Flower(get_pos() + Vector(0, -32), Flower::FIREFLOWER));
        sector->add_object(riser);
      }
      SoundManager::get()->play_sound(IDToSound(SND_UPGRADE));
      break;

    case 5: // grow/iceflower
      if(player.size == SMALL) {
        SpecialRiser* riser = new SpecialRiser(
            new GrowUp(get_pos() + Vector(0, -32)));
        sector->add_object(riser);                                            
      } else {
        SpecialRiser* riser = new SpecialRiser(                               
            new Flower(get_pos() + Vector(0, -32), Flower::ICEFLOWER));
        sector->add_object(riser);
      }      
      SoundManager::get()->play_sound(IDToSound(SND_UPGRADE));
      break;

    case 3: // star
      sector->add_object(new Star(get_pos() + Vector(0, -32)));
      break;

    case 4: // 1up
      sector->add_object(new OneUp(get_pos()));
      break;

    default:
      assert(false);
  }

  start_bounce();
  sprite->set_action("empty");
}

//IMPLEMENT_FACTORY(BonusBlock, "bonusblock")

//---------------------------------------------------------------------------

Brick::Brick(const Vector& pos, int data)
  : Block(pos, sprite_manager->create("brick")), breakable(false),
    coin_counter(0)
{
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
  
  SoundManager::get()->play_sound(IDToSound(SND_BRICK));
  Sector* sector = Sector::current();
  Player& player = *(sector->player);
  if(coin_counter > 0) {
    sector->add_object(new BouncyCoin(get_pos()));
    coin_counter--;
    player.get_status().incCoins();
    if(coin_counter == 0)
      sprite->set_action("empty");
    start_bounce();
  } else if(breakable) {
    if(playerhit && player.size == SMALL) {
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
