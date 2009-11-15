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

//#include <config.h>

#include "audio/sound_manager.hpp"
#include "object/invisible_block.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/constants.hpp"
//#include "supertux/object_factory.hpp"
//#include "supertux/resources.hpp"
//#include "video/drawing_context.hpp"

InvisibleBlock::InvisibleBlock(const Vector& pos)
  : Block(sprite_manager->create("images/objects/bonus_block/invisibleblock.sprite")), visible(false)
{
  bbox.set_pos(pos);
  sound_manager->preload("sounds/brick.wav");
}

void
InvisibleBlock::draw(DrawingContext& context)
{
  if(visible)
    sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

bool
InvisibleBlock::collides(GameObject& other, const CollisionHit& )
{
  if(visible)
    return true;

  // if we're not visible, only register a collision if this will make us visible
  Player* player = dynamic_cast<Player*> (&other);
  if ((player) 
          && (player->get_movement().y <= 0)
          && (player->get_bbox().get_top() > get_bbox().get_bottom() - SHIFT_DELTA)) {
    return true;
  }

  return false;
}

HitResponse
InvisibleBlock::collision(GameObject& other, const CollisionHit& hit)
{
  return Block::collision(other, hit);
}

void
InvisibleBlock::hit(Player& player)
{
  sound_manager->play("sounds/brick.wav");

  if(visible)
    return;

  sprite->set_action("empty");
  start_bounce(&player);
  set_group(COLGROUP_STATIC);
  visible = true;
}

//IMPLEMENT_FACTORY(InvisibleBlock, "invisible_block");
