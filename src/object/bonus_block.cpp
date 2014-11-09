//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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
#include "object/flower.hpp"
#include "object/bouncy_coin.hpp"
#include "object/coin_explode.hpp"
#include "object/coin_rain.hpp"
#include "object/growup.hpp"
#include "object/oneup.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "object/powerup.hpp"
#include "object/specialriser.hpp"
#include "object/star.hpp"
#include "object/trampoline.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/constants.hpp"
#include "supertux/level.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

#include <stdexcept>

BonusBlock::BonusBlock(const Vector& pos, int data) :
  Block(SpriteManager::current()->create("images/objects/bonus_block/bonusblock.sprite")),
  contents(),
  object(),
  hit_counter(1),
  sprite_name(),
  script(),
  lightsprite()
{
  bbox.set_pos(pos);
  sprite->set_action("normal");
  switch(data) {
    case 1: contents = CONTENT_COIN; break;
    case 2: contents = CONTENT_FIREGROW; break;
    case 3: contents = CONTENT_STAR; break;
    case 4: contents = CONTENT_1UP; break;
    case 5: contents = CONTENT_ICEGROW; break;
    case 6: contents = CONTENT_LIGHT;
      SoundManager::current()->preload("sounds/switch.ogg");
      lightsprite=Surface::create("/images/objects/lightmap_light/bonusblock_light.png");
      break;
    case 7: contents = CONTENT_TRAMPOLINE;
      //object = new Trampoline(get_pos(), false); //needed if this is to be moved to custom
      break;
    case 8: contents = CONTENT_CUSTOM;
      object = std::make_shared<Trampoline>(get_pos(), true);
      break;
    case 9: contents = CONTENT_CUSTOM;
      object = std::make_shared<Rock>(get_pos(), "images/objects/rock/rock.sprite");
      break;
    case 10: contents = CONTENT_RAIN; break;
    case 11: contents = CONTENT_EXPLODE; break;
    case 12: contents = CONTENT_CUSTOM;
      object = std::make_shared<PowerUp>(get_pos(), "images/powerups/potions/red-potion.sprite");
      break;
    case 13: contents = CONTENT_AIRGROW; break;
    case 14: contents = CONTENT_EARTHGROW; break;
    default:
      log_warning << "Invalid box contents" << std::endl;
      contents = CONTENT_COIN;
      break;
  }
}

BonusBlock::BonusBlock(const Reader& lisp) :
  Block(SpriteManager::current()->create("images/objects/bonus_block/bonusblock.sprite")),
  contents(),
  object(0),
  hit_counter(1),
  sprite_name(),
  script(),
  lightsprite()
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
    } else if(token == "sprite") {
      iter.value()->get(sprite_name);
      sprite = SpriteManager::current()->create(sprite_name);
    } else if(token == "count") {
      iter.value()->get(hit_counter);
    } else if(token == "script") {
      iter.value()->get(script);
    } else if(token == "contents") {
      std::string contentstring;
      iter.value()->get(contentstring);
      if(contentstring == "coin") {
        contents = CONTENT_COIN;
      } else if(contentstring == "firegrow") {
        contents = CONTENT_FIREGROW;
      } else if(contentstring == "icegrow") {
        contents = CONTENT_ICEGROW;
      } else if(contentstring == "airgrow") {
        contents = CONTENT_AIRGROW;
      } else if(contentstring == "earthgrow") {
        contents = CONTENT_EARTHGROW;
      } else if(contentstring == "star") {
        contents = CONTENT_STAR;
      } else if(contentstring == "1up") {
        contents = CONTENT_1UP;
      } else if(contentstring == "custom") {
        contents = CONTENT_CUSTOM;
      } else if(contentstring == "script") { // use when bonusblock is to contain ONLY a script
        contents = CONTENT_SCRIPT;
      } else if(contentstring == "light") {
        contents = CONTENT_LIGHT;
        SoundManager::current()->preload("sounds/switch.ogg");
      } else if(contentstring == "trampoline") {
        contents = CONTENT_TRAMPOLINE;
      } else if(contentstring == "rain") {
        contents = CONTENT_RAIN;
      } else if(contentstring == "explode") {
        contents = CONTENT_EXPLODE;
      } else {
        log_warning << "Invalid box contents '" << contentstring << "'" << std::endl;
      }
    } else {
      if(contents == CONTENT_CUSTOM) {
        GameObjectPtr game_object = ObjectFactory::instance().create(token, *(iter.lisp()));
        object = std::dynamic_pointer_cast<MovingObject>(game_object);
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
  if(contents == CONTENT_LIGHT)
    lightsprite = Surface::create("/images/objects/lightmap_light/bonusblock_light.png");

  bbox.set_pos(pos);
}

BonusBlock::~BonusBlock()
{
}

void
BonusBlock::hit(Player & player)
{
  try_open(&player);
}

HitResponse
BonusBlock::collision(GameObject& other, const CollisionHit& hit_){

  Player* player = dynamic_cast<Player*> (&other);
  if (player) {
    if (player->does_buttjump)
      try_drop(player);
  }

  BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
  if(badguy) {
    // hit contains no information for collisions with blocks.
    // Badguy's bottom has to be below the top of the block
    // SHIFT_DELTA is required to slide over one tile gaps.
    if( badguy->can_break() && ( badguy->get_bbox().get_bottom() > get_bbox().get_top() + SHIFT_DELTA ) ){
      try_open(player);
    }
  }
  Portable* portable = dynamic_cast<Portable*> (&other);
  if(portable) {
    MovingObject* moving = dynamic_cast<MovingObject*> (&other);
    if(moving->get_bbox().get_top() > get_bbox().get_bottom() - SHIFT_DELTA) {
      try_open(player);
    }
  }
  return Block::collision(other, hit_);
}

void
BonusBlock::try_open(Player *player)
{
  if(sprite->get_action() == "empty") {
    SoundManager::current()->play("sounds/brick.wav");
    return;
  }

  Sector* sector = Sector::current();
  assert(sector);

  if (player == NULL)
    player = sector->player;

  if (player == NULL)
    return;

  Direction direction = (player->get_bbox().get_middle().x > get_bbox().get_middle().x) ? LEFT : RIGHT;

  switch(contents) {
    case CONTENT_COIN:
    {
      Sector::current()->add_object(std::make_shared<BouncyCoin>(get_pos(), true));
      player->get_status()->add_coins(1);
      if (hit_counter != 0)
        Sector::current()->get_level()->stats.coins++;
      break;
    }

    case CONTENT_FIREGROW:
    {
      if(player->get_status()->bonus == NO_BONUS) {
        auto riser = std::make_shared<SpecialRiser>(get_pos(), std::make_shared<GrowUp>(direction));
        sector->add_object(riser);
      } else {
        auto riser = std::make_shared<SpecialRiser>(
          get_pos(), std::make_shared<Flower>(FIRE_BONUS));
        sector->add_object(riser);
      }
      SoundManager::current()->play("sounds/upgrade.wav");
      break;
    }

    case CONTENT_ICEGROW:
    {
      if(player->get_status()->bonus == NO_BONUS) {
        auto riser = std::make_shared<SpecialRiser>(get_pos(), std::make_shared<GrowUp>(direction));
        sector->add_object(riser);
      } else {
        auto riser = std::make_shared<SpecialRiser>(
          get_pos(), std::make_shared<Flower>(ICE_BONUS));
        sector->add_object(riser);
      }
      SoundManager::current()->play("sounds/upgrade.wav");
      break;
    }

    case CONTENT_AIRGROW:
    {
      if(player->get_status()->bonus == NO_BONUS) {
        auto riser = std::make_shared<SpecialRiser>(get_pos(), std::make_shared<GrowUp>(direction));
        sector->add_object(riser);
      } else {
        auto riser = std::make_shared<SpecialRiser>(
          get_pos(), std::make_shared<Flower>(AIR_BONUS));
        sector->add_object(riser);
      }
      SoundManager::current()->play("sounds/upgrade.wav");
      break;
    }

    case CONTENT_EARTHGROW:
    {
      if(player->get_status()->bonus == NO_BONUS) {
        auto riser = std::make_shared<SpecialRiser>(get_pos(), std::make_shared<GrowUp>(direction));
        sector->add_object(riser);
      } else {
        auto riser = std::make_shared<SpecialRiser>(
          get_pos(), std::make_shared<Flower>(EARTH_BONUS));
        sector->add_object(riser);
      }
      SoundManager::current()->play("sounds/upgrade.wav");
      break;
    }

    case CONTENT_STAR:
    {
      sector->add_object(std::make_shared<Star>(get_pos() + Vector(0, -32), direction));
      SoundManager::current()->play("sounds/upgrade.wav");
      break;
    }

    case CONTENT_1UP:
    {
      sector->add_object(std::make_shared<OneUp>(get_pos(), direction));
      SoundManager::current()->play("sounds/upgrade.wav");
      break;
    }

    case CONTENT_CUSTOM:
    {
      auto riser = std::make_shared<SpecialRiser>(get_pos(), object);
      object = 0;
      sector->add_object(riser);
      SoundManager::current()->play("sounds/upgrade.wav");
      break;
    }

    case CONTENT_SCRIPT:
    { break; } // because scripts always run, this prevents default contents from being assumed

    case CONTENT_LIGHT:
    {
      if(sprite->get_action() == "on")
        sprite->set_action("off");
      else
        sprite->set_action("on");
      SoundManager::current()->play("sounds/switch.ogg");
      break;
    }
    case CONTENT_TRAMPOLINE:
    {
      auto riser = std::make_shared<SpecialRiser>(get_pos(), std::make_shared<Trampoline>(get_pos(), false));
      sector->add_object(riser);
      SoundManager::current()->play("sounds/upgrade.wav");
      break;
    }
    case CONTENT_RAIN:
    {
      hit_counter = 1; // multiple hits of coin rain is not allowed
      Sector::current()->add_object(std::make_shared<CoinRain>(get_pos(), true));
      SoundManager::current()->play("sounds/upgrade.wav");
      break;
    }
    case CONTENT_EXPLODE:
    {
      hit_counter = 1; // multiple hits of coin explode is not allowed
      Sector::current()->add_object(std::make_shared<CoinExplode>(get_pos() + Vector (0, -40)));
      SoundManager::current()->play("sounds/upgrade.wav");
      break;
    }
  }

  if(script != "") { // scripts always run if defined
    std::istringstream stream(script);
    Sector::current()->run_script(stream, "BonusBlockScript");
  }

  start_bounce(player);
  if(hit_counter <= 0 || contents == CONTENT_LIGHT){ //use 0 to allow infinite hits
  }else if(hit_counter == 1){
    sprite->set_action("empty");
  }else{
    hit_counter--;
  }
}

void
BonusBlock::try_drop(Player *player)
{
  if(sprite->get_action() == "empty") {
    SoundManager::current()->play("sounds/brick.wav");
    return;
  }

  Sector* sector = Sector::current();
  assert(sector);

  // First what's below the bonus block, if solid send it up anyway (excepting doll)
  Rectf dest_;
  dest_.p1.x = bbox.get_left() + 1;
  dest_.p1.y = bbox.get_bottom() + 1;
  dest_.p2.x = bbox.get_right() - 1;
  dest_.p2.y = dest_.p1.y + 30;
  if (!Sector::current()->is_free_of_statics(dest_, this, true) && !(contents == CONTENT_1UP)) {
    try_open(player);
    return;
  }

  if (player == NULL)
    player = sector->player;

  if (player == NULL)
    return;

  Direction direction = (player->get_bbox().get_middle().x > get_bbox().get_middle().x) ? LEFT : RIGHT;

  bool countdown = false;

  switch(contents) {
    case CONTENT_COIN:
    {
      try_open(player);
      break;
    }

    case CONTENT_FIREGROW:
    {
      sector->add_object(std::make_shared<PowerUp>(get_pos() + Vector(0, 32), "images/powerups/fireflower/fireflower.sprite"));
      SoundManager::current()->play("sounds/upgrade.wav");
      countdown = true;
      break;
    }

    case CONTENT_ICEGROW:
    {
      sector->add_object(std::make_shared<PowerUp>(get_pos() + Vector(0, 32), "images/powerups/iceflower/iceflower.sprite"));
      SoundManager::current()->play("sounds/upgrade.wav");
      countdown = true;
      break;
    }

    case CONTENT_AIRGROW:
    {
      sector->add_object(std::make_shared<PowerUp>(get_pos() + Vector(0, 32), "images/powerups/airflower/airflower.sprite"));
      SoundManager::current()->play("sounds/upgrade.wav");
      countdown = true;
      break;
    }

    case CONTENT_EARTHGROW:
    {
      sector->add_object(std::make_shared<PowerUp>(get_pos() + Vector(0, 32), "images/powerups/earthflower/earthflower.sprite"));
      SoundManager::current()->play("sounds/upgrade.wav");
      countdown = true;
      break;
    }

    case CONTENT_STAR:
    {
      sector->add_object(std::make_shared<Star>(get_pos() + Vector(0, 32), direction));
      SoundManager::current()->play("sounds/upgrade.wav");
      countdown = true;
      break;
    }

    case CONTENT_1UP:
    {
      sector->add_object(std::make_shared<OneUp>(get_pos(), DOWN));
      SoundManager::current()->play("sounds/upgrade.wav");
      countdown = true;
      break;
    }

    case CONTENT_CUSTOM:
    {
      //NOTE: non-portable trampolines could be moved to CONTENT_CUSTOM, but they should not drop
      object->set_pos(get_pos() +  Vector(0, 32));
      sector->add_object(object);
      object = 0;
      SoundManager::current()->play("sounds/upgrade.wav");
      countdown = true;
      break;
    }

    case CONTENT_SCRIPT:
    { break; } // because scripts always run, this prevents default contents from being assumed

    case CONTENT_LIGHT:
    {
      try_open(player);
      break;
    }
    case CONTENT_TRAMPOLINE:
    {
      try_open(player);
      break;
    }
    case CONTENT_RAIN:
    {
      try_open(player);
      break;
    }
    case CONTENT_EXPLODE:
    {
      hit_counter = 1; // multiple hits of coin explode is not allowed
      Sector::current()->add_object(std::make_shared<CoinExplode>(get_pos() + Vector (0, 40)));
      SoundManager::current()->play("sounds/upgrade.wav");
      countdown = true;
      break;
    }
  }

  if(script != "") { // scripts always run if defined
    std::istringstream stream(script);
    Sector::current()->run_script(stream, "powerup-script");
  }

  if(countdown){ // only decrease hit counter if try_open was not called
    if(hit_counter == 1){
      sprite->set_action("empty");
    }else{
      hit_counter--;
    }
  }
}

void
BonusBlock::draw(DrawingContext& context){
  // do the regular drawing first
  Block::draw(context);
  // then Draw the light if on.
  if(sprite->get_action() == "on") {
    Vector pos = get_pos() + (bbox.get_size().as_vector() - lightsprite->get_size()) / 2;
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    context.draw_surface(lightsprite, pos, 10);
    context.pop_target();
  }
}
/* EOF */
