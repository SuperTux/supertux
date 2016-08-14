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

#include "object/brick.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "object/bouncy_coin.hpp"
#include "object/explosion.hpp"
#include "object/flower.hpp"
#include "object/icecrusher.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/constants.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

Brick::Brick(const Vector& pos, int data, const std::string& spriteName)
  : Block(SpriteManager::current()->create(spriteName)), breakable(false),
    coin_counter(0)
{
  bbox.set_pos(pos);
  if(data == 1)
    coin_counter = 5;
  else
    breakable = true;
}

Brick::Brick(const ReaderMapping& lisp) :
  Block(lisp, "images/objects/bonus_block/brick.sprite"),
  breakable(),
  coin_counter(0)
{
  if (!lisp.get("breakable",breakable)) breakable = true;
  if (!breakable) {
    coin_counter = 5;
  }
}

void
Brick::hit(Player& player)
{
  if(sprite->get_action() == "empty")
    return;

  try_break(&player);
}

HitResponse
Brick::collision(GameObject& other, const CollisionHit& hit_){

  auto player = dynamic_cast<Player*> (&other);
  if (player) {
    if (player->does_buttjump) try_break(player);
    if (player->is_stone() && player->get_velocity().y >= 280) try_break(player); // stoneform breaks through bricks
  }

  auto badguy = dynamic_cast<BadGuy*> (&other);
  if(badguy) {
    // hit contains no information for collisions with blocks.
    // Badguy's bottom has to be below the top of the brick
    // SHIFT_DELTA is required to slide over one tile gaps.
    if( badguy->can_break() && ( badguy->get_bbox().get_bottom() > bbox.get_top() + SHIFT_DELTA ) ){
      try_break(player);
    }
  }
  auto portable = dynamic_cast<Portable*> (&other);
  if(portable) {
    auto moving = dynamic_cast<MovingObject*> (&other);
    if(moving->get_bbox().get_top() > bbox.get_bottom() - SHIFT_DELTA) {
      try_break(player);
    }
  }
  auto explosion = dynamic_cast<Explosion*> (&other);
  if(explosion && explosion->hurts()) {
    try_break(player);
  }
  auto icecrusher = dynamic_cast<IceCrusher*> (&other);
  if(icecrusher && coin_counter == 0)
    try_break(player);
  return Block::collision(other, hit_);
}

void
Brick::try_break(Player* player)
{
  if(sprite->get_action() == "empty")
    return;

  SoundManager::current()->play("sounds/brick.wav");
  auto sector = Sector::current();
  auto& player_one = *(sector->player);
  if(coin_counter > 0 ){
    sector->add_object(std::make_shared<BouncyCoin>(get_pos(), true));
    coin_counter--;
    player_one.get_status()->add_coins(1);
    if(coin_counter == 0)
      sprite->set_action("empty");
    start_bounce(player);
  } else if(breakable) {
    if(player){
      if(player->is_big()){
        start_break(player);
        return;
      } else {
        start_bounce(player);
        return;
      }
    }
    break_me();
  }
}

ObjectSettings
Brick::get_settings() {
  ObjectSettings result = Block::get_settings();

  result.options.push_back(ObjectOption(MN_TOGGLE, _("Breakable"), &breakable,
                                        "breakable"));

  return result;
}

//IMPLEMENT_FACTORY(Brick, "brick");

/* EOF */
