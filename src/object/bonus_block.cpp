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
#include "util/reader_mapping.hpp"

#include <stdexcept>
#include <physfs.h>

BonusBlock::BonusBlock(const Vector& pos, int data) :
  Block(SpriteManager::current()->create("images/objects/bonus_block/bonusblock.sprite")),
  contents(),
  object(),
  hit_counter(1),
  script(),
  lightsprite()
{
  bbox.set_pos(pos);
  sprite->set_action("normal");
  get_content_by_data(data);
}

BonusBlock::BonusBlock(const ReaderMapping& lisp) :
  Block(lisp, "images/objects/bonus_block/bonusblock.sprite"),
  contents(),
  object(0),
  hit_counter(1),
  script(),
  lightsprite()
{
  contents = CONTENT_COIN;
  auto iter = lisp.get_iter();
  while(iter.next()) {
    const std::string& token = iter.get_key();
    if(token == "x" || token == "y" || token == "sprite") {
      // already initialized in Block::Block
    } else if(token == "count") {
      iter.get(hit_counter);
    } else if(token == "script") {
      iter.get(script);
    } else if(token == "data") {
      int d = 0;
      iter.get(d);
      get_content_by_data(d);
    } else if(token == "contents") {
      std::string contentstring;
      iter.get(contentstring);
      contents = get_content_from_string(contentstring);
    } else {
      if(contents == CONTENT_CUSTOM) {
        ReaderMapping object_mapping = iter.as_mapping();
        GameObjectPtr game_object = ObjectFactory::instance().create(token, object_mapping);
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
  if(contents == CONTENT_LIGHT) {
    SoundManager::current()->preload("sounds/switch.ogg");
    lightsprite = Surface::create("/images/objects/lightmap_light/bonusblock_light.png");
  }
}

void
BonusBlock::get_content_by_data(int d)
{
  switch(d) {
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

BonusBlock::~BonusBlock()
{
}

void
BonusBlock::save(Writer& writer) {
  Block::save(writer);
  writer.write("contents", contents_to_string(contents), false);
  if(contents == CONTENT_CUSTOM && object)
  {
    writer.start_list(object->get_class());
    object->save(writer);
    writer.end_list(object->get_class());
  }
  if (script != "") {
    writer.write("script", script, false);
  }
  if (hit_counter != 1) {
    writer.write("count", hit_counter);
  }
}

ObjectSettings
BonusBlock::get_settings() {
  ObjectSettings result = Block::get_settings();
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Script"), &script));
  result.options.push_back( ObjectOption(MN_INTFIELD, _("Count"), &hit_counter));

  ObjectOption coo(MN_STRINGSELECT, _("Content"), &contents);
  coo.select.push_back(_("coin"));
  coo.select.push_back(_("Growth (fire flower)"));
  coo.select.push_back(_("Growth (ice flower)"));
  coo.select.push_back(_("Growth (air flower)"));
  coo.select.push_back(_("Growth (earth flower)"));
  coo.select.push_back(_("star"));
  coo.select.push_back(_("Tux doll"));
  coo.select.push_back(_("custom"));
  coo.select.push_back(_("script"));
  coo.select.push_back(_("light"));
  coo.select.push_back(_("trampoline"));
  coo.select.push_back(_("coin rain"));
  coo.select.push_back(_("coin explosion"));
  result.options.push_back(coo);

  return result;
}


void
BonusBlock::hit(Player & player)
{
  try_open(&player);
}

HitResponse
BonusBlock::collision(GameObject& other, const CollisionHit& hit_){

  auto player = dynamic_cast<Player*> (&other);
  if (player) {
    if (player->does_buttjump)
      try_drop(player);
  }

  auto badguy = dynamic_cast<BadGuy*> (&other);
  if(badguy) {
    // hit contains no information for collisions with blocks.
    // Badguy's bottom has to be below the top of the block
    // SHIFT_DELTA is required to slide over one tile gaps.
    if( badguy->can_break() && ( badguy->get_bbox().get_bottom() > bbox.get_top() + SHIFT_DELTA ) ){
      try_open(player);
    }
  }
  auto portable = dynamic_cast<Portable*> (&other);
  if(portable) {
    auto moving = dynamic_cast<MovingObject*> (&other);
    if(moving->get_bbox().get_top() > bbox.get_bottom() - SHIFT_DELTA) {
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

  auto sector = Sector::current();
  assert(sector);

  if (player == NULL)
    player = sector->player;

  if (player == NULL)
    return;

  Direction direction = (player->get_bbox().get_middle().x > bbox.get_middle().x) ? LEFT : RIGHT;

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
      raise_growup_bonus(player, FIRE_BONUS, direction);
      break;
    }

    case CONTENT_ICEGROW:
    {
      raise_growup_bonus(player, ICE_BONUS, direction);
      break;
    }

    case CONTENT_AIRGROW:
    {
      raise_growup_bonus(player, AIR_BONUS, direction);
      break;
    }

    case CONTENT_EARTHGROW:
    {
      raise_growup_bonus(player, EARTH_BONUS, direction);
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

  if(!script.empty()) { // scripts always run if defined
    Sector::current()->run_script(script, "BonusBlockScript");
  }

  start_bounce(player);
  if(hit_counter <= 0 || contents == CONTENT_LIGHT){ //use 0 to allow infinite hits
  }else if(hit_counter == 1){
    sprite->set_action("empty");
  } else {
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

  auto sector = Sector::current();
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

  Direction direction = (player->get_bbox().get_middle().x > bbox.get_middle().x) ? LEFT : RIGHT;

  bool countdown = false;

  switch(contents) {
    case CONTENT_COIN:
    {
      try_open(player);
      break;
    }

    case CONTENT_FIREGROW:
    {
      drop_growup_bonus("images/powerups/fireflower/fireflower.sprite", countdown);
      break;
    }

    case CONTENT_ICEGROW:
    {
      drop_growup_bonus("images/powerups/iceflower/iceflower.sprite", countdown);
      break;
    }

    case CONTENT_AIRGROW:
    {
      drop_growup_bonus("images/powerups/airflower/airflower.sprite", countdown);
      break;
    }

    case CONTENT_EARTHGROW:
    {
      drop_growup_bonus("images/powerups/earthflower/earthflower.sprite", countdown);
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
    {
      countdown = true;
      break;
    } // because scripts always run, this prevents default contents from being assumed

    case CONTENT_LIGHT:
    case CONTENT_TRAMPOLINE:
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

  if(!script.empty()) { // scripts always run if defined
    Sector::current()->run_script(script, "powerup-script");
  }

  if(countdown){ // only decrease hit counter if try_open was not called
    if(hit_counter == 1){
      sprite->set_action("empty");
    } else {
      hit_counter--;
    }
  }
}

void
BonusBlock::raise_growup_bonus(Player* player, const BonusType& bonus, const Direction& dir)
{
  std::shared_ptr<MovingObject> obj;
  if(player->get_status()->bonus == NO_BONUS) {
    obj = std::make_shared<GrowUp>(dir);
  } else {
    obj = std::make_shared<Flower>(bonus);
  }

  auto riser = std::make_shared<SpecialRiser>(get_pos(), obj);
  Sector::current()->add_object(riser);
  SoundManager::current()->play("sounds/upgrade.wav");
}

void
BonusBlock::drop_growup_bonus(const std::string& bonus_sprite_name, bool& countdown)
{
  Sector::current()->add_object(std::make_shared<PowerUp>(get_pos() + Vector(0, 32), bonus_sprite_name));
  SoundManager::current()->play("sounds/upgrade.wav");
  countdown = true;
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

BonusBlock::Contents
BonusBlock::get_content_from_string(const std::string& contentstring) const
{
  if(contentstring == "coin")
    return CONTENT_COIN;
  if(contentstring == "firegrow")
    return CONTENT_FIREGROW;
  if(contentstring == "icegrow")
    return CONTENT_ICEGROW;
  if(contentstring == "airgrow")
    return CONTENT_AIRGROW;
  if(contentstring == "earthgrow")
    return CONTENT_EARTHGROW;
  if(contentstring == "star")
    return CONTENT_STAR;
  if(contentstring == "1up")
    return CONTENT_1UP;
  if(contentstring == "custom")
    return CONTENT_CUSTOM;
  if(contentstring == "script") // use when bonusblock is to contain ONLY a script
    return CONTENT_SCRIPT;
  if(contentstring == "light")
    return CONTENT_LIGHT;
  if(contentstring == "trampoline")
    return CONTENT_TRAMPOLINE;
  if(contentstring == "rain")
    return CONTENT_RAIN;
  if(contentstring == "explode")
    return CONTENT_EXPLODE;

  log_warning << "Invalid box contents '" << contentstring << "'" << std::endl;
  return CONTENT_COIN;
}

std::string
BonusBlock::contents_to_string(const BonusBlock::Contents& content) const
{
  switch(contents)
  {
    case CONTENT_COIN:
      return "coin";
    case CONTENT_FIREGROW:
      return "firegrow";
    case CONTENT_ICEGROW:
      return "icegrow";
    case CONTENT_AIRGROW:
      return "airgrow";
    case CONTENT_EARTHGROW:
      return "earthgrow";
    case CONTENT_STAR:
      return "star";
    case CONTENT_1UP:
      return "1up";
    case CONTENT_CUSTOM:
      return "custom";
    case CONTENT_SCRIPT:
      return "script";
    case CONTENT_LIGHT:
      return "light";
    case CONTENT_TRAMPOLINE:
      return "trampoline";
    case CONTENT_RAIN:
      return "rain";
    case CONTENT_EXPLODE:
      return "explode";
  }
  return "coin";
}
/* EOF */
