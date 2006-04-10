//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include "lisp/writer.hpp"
#include "lisp/lisp.hpp"
#include "player_status.hpp"
#include "resources.hpp"
#include "gettext.hpp"
#include "video/drawing_context.hpp"
#include "audio/sound_manager.hpp"
#include "sprite/sprite_manager.hpp"
#include "math/vector.hpp"
#include "main.hpp"
#include "msg.hpp"

static const int START_COINS = 100;
static const int MAX_COINS = 99999;

PlayerStatus* player_status = 0;

PlayerStatus::PlayerStatus()
  : coins(START_COINS),
    bonus(NO_BONUS),
    score_multiplier(1),
    max_score_multiplier(1)
{
  reset();
  key_brass.reset(sprite_manager->create("images/objects/keys/key_brass.sprite"));
  key_iron.reset(sprite_manager->create("images/objects/keys/key_iron.sprite"));
  key_bronze.reset(sprite_manager->create("images/objects/keys/key_bronze.sprite"));
  key_silver.reset(sprite_manager->create("images/objects/keys/key_silver.sprite"));
  key_gold.reset(sprite_manager->create("images/objects/keys/key_gold.sprite"));
  key_brass->set_action("outline");
  key_iron->set_action("outline");
  key_bronze->set_action("outline");
  key_silver->set_action("outline");
  key_gold->set_action("outline");
  
  tux_life.reset(sprite_manager->create("images/creatures/tux_small/tux-life.sprite"));

  Console::registerCommand("coins", this);
}

PlayerStatus::~PlayerStatus()
{
}

void PlayerStatus::reset()
{
  coins = START_COINS;
  keys = 0;
  bonus = NO_BONUS;
  score_multiplier = 1;
  max_score_multiplier = 1;
}

void
PlayerStatus::incLives()
{
  player_status->coins = std::min(player_status->coins+100, MAX_COINS);
  sound_manager->play("sounds/lifeup.wav");
}

void
PlayerStatus::incCoins()
{
  coins++;
  sound_manager->play("sounds/coin.wav");
}

void
PlayerStatus::set_keys(int new_key)
{
  keys |= new_key;
  key_brass->set_action(keys & KEY_BRASS ? "display" : "outline");
  key_iron->set_action(keys & KEY_IRON ? "display" : "outline");
  key_bronze->set_action(keys & KEY_BRONZE ? "display" : "outline");
  key_silver->set_action(keys & KEY_SILVER ? "display" : "outline");
  key_gold->set_action(keys & KEY_GOLD ? "display" : "outline");
}

void
PlayerStatus::write(lisp::Writer& writer)
{
  switch(bonus) {
    case NO_BONUS:
      writer.write_string("bonus", "none");
      break;
    case GROWUP_BONUS:
      writer.write_string("bonus", "growup");
      break;
    case FIRE_BONUS:
      writer.write_string("bonus", "fireflower");
      break;
    case ICE_BONUS:
      writer.write_string("bonus", "iceflower");
      break;
    default:
      msg_warning << "Unknown bonus type." << std::endl;
      writer.write_string("bonus", "none");
  }
  writer.write_bool("key-brass", keys & KEY_BRASS);
  writer.write_bool("key-iron", keys & KEY_IRON);
  writer.write_bool("key-bronze", keys & KEY_BRONZE);
  writer.write_bool("key-silver", keys & KEY_SILVER);
  writer.write_bool("key-gold", keys & KEY_GOLD);

  writer.write_int("coins", coins);
  writer.write_int("max-score-multiplier", max_score_multiplier);
}

void
PlayerStatus::read(const lisp::Lisp& lisp)
{
  reset();
  
  std::string bonusname;
  if(lisp.get("bonus", bonusname)) {
    if(bonusname == "none") {
      bonus = NO_BONUS;
    } else if(bonusname == "growup") {
      bonus = GROWUP_BONUS;
    } else if(bonusname == "fireflower") {
      bonus = FIRE_BONUS;
    } else if(bonusname == "iceflower") {
      bonus = ICE_BONUS;
    } else {
      msg_warning << "Unknown bonus '" << bonusname << "' in savefile" << std::endl;
      bonus = NO_BONUS;
    }
  }
  bool val = false;
  if(lisp.get("key-brass", val) && val == true)
    set_keys(KEY_BRASS);
  if(lisp.get("key-iron", val) && val == true)
    set_keys(KEY_IRON);
  if(lisp.get("key-bronze", val) && val == true)
    set_keys(KEY_BRONZE);
  if(lisp.get("key-silver", val) && val == true)
    set_keys(KEY_SILVER);
  if(lisp.get("key-gold", val) && val == true)
    set_keys(KEY_GOLD);

  lisp.get("coins", coins);
  lisp.get("max-score-multiplier", max_score_multiplier);
}

void
PlayerStatus::draw_keys(DrawingContext& context)
{
  const float SPACING = 10;
  float x,y; 
  x = BORDER_X; y = BORDER_Y;
  key_brass->draw(context, Vector(x, y), LAYER_FOREGROUND1);
  x += key_brass->get_width() + SPACING;
  key_iron->draw(context, Vector(x, y), LAYER_FOREGROUND1);
  x += key_iron->get_width() + SPACING;
  key_bronze->draw(context, Vector(x, y), LAYER_FOREGROUND1);
  x += key_bronze->get_width() + SPACING;
  key_silver->draw(context, Vector(x, y), LAYER_FOREGROUND1);
  x += key_silver->get_width() + SPACING;
  key_gold->draw(context, Vector(x, y), LAYER_FOREGROUND1);
  x += key_gold->get_width() + SPACING;
}

void
PlayerStatus::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));

  char str[60];
 
  int displayCoins = std::max(player_status->coins, 0);
  sprintf(str, "%d", displayCoins);
  const char* coinstext = _("COINS");
  context.draw_text(white_text, coinstext, Vector(SCREEN_WIDTH - white_text->get_text_width(coinstext) - gold_text->get_text_width(" 99999") - BORDER_X, BORDER_Y), LEFT_ALLIGN, LAYER_FOREGROUND1);
  context.draw_text(gold_text, str, Vector(SCREEN_WIDTH - BORDER_X, BORDER_Y), RIGHT_ALLIGN, LAYER_FOREGROUND1);

  draw_keys(context);  

  context.pop_transform();
}

void
PlayerStatus::operator= (const PlayerStatus& other)
{
  coins = other.coins;
  bonus = other.bonus;
  score_multiplier = other.score_multiplier;
  max_score_multiplier = other.max_score_multiplier;
  keys = other.keys;
}

bool
PlayerStatus::consoleCommand(std::string command, std::vector<std::string> arguments)
{
  if (command == "coins") {
    if ((arguments.size() < 1) || (!Console::string_is<int>(arguments[0]))) {
      msg_info << "Usage: coins <number>" << std::endl;
    } else {
      coins = Console::string_to<int>(arguments[0]);
    }
    return true;
  }
  return false;
}

