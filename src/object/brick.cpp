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

#include "object/brick.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "object/flower.hpp"
#include "object/bouncy_coin.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/constants.hpp"
#include "supertux/sector.hpp"

Brick::Brick(const Vector& pos, int data)
  : Block(sprite_manager->create("images/objects/bonus_block/brick.sprite")), breakable(false),
    coin_counter(0)
{
  bbox.set_pos(pos);
  if(data == 1)
    coin_counter = 5;
  else
    breakable = true;
}

void
Brick::hit(Player& player)
{
  if(sprite->get_action() == "empty")
    return;

  try_break(&player);
}

HitResponse
Brick::collision(GameObject& other, const CollisionHit& hit){

  Player* player = dynamic_cast<Player*> (&other);
  if (player) {
    if (player->does_buttjump) try_break(player);
  }

  BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
  if(badguy) {
    // hit contains no information for collisions with blocks.
    // Badguy's bottom has to be below the top of the brick
    // SHIFT_DELTA is required to slide over one tile gaps.
    if( badguy->can_break() && ( badguy->get_bbox().get_bottom() > get_bbox().get_top() + SHIFT_DELTA ) ){
      try_break(player);
    }
  }
  Portable* portable = dynamic_cast<Portable*> (&other);
  if(portable) {
    MovingObject* moving = dynamic_cast<MovingObject*> (&other);
    if(moving->get_bbox().get_top() > get_bbox().get_bottom() - SHIFT_DELTA) {
      try_break(player);
    }
  }
  return Block::collision(other, hit);
}

void
Brick::try_break(Player* player)
{
  if(sprite->get_action() == "empty")
    return;

  sound_manager->play("sounds/brick.wav");
  Sector* sector = Sector::current();
  Player& player_one = *(sector->player);
  if(coin_counter > 0) {
    sector->add_object(new BouncyCoin(get_pos(),true));
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

//IMPLEMENT_FACTORY(Brick, "brick");

/* EOF */
