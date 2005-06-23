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

#include "invisible_block.h"
#include "resources.h"
#include "sprite/sprite.h"
#include "sprite/sprite_manager.h"
#include "video/drawing_context.h"
#include "audio/sound_manager.h"
#include "object_factory.h"

InvisibleBlock::InvisibleBlock(const Vector& pos)
  : Block(sprite_manager->create("invisibleblock")), visible(false)
{
  bbox.set_pos(pos);
  flags &= ~FLAG_SOLID;
}

void
InvisibleBlock::draw(DrawingContext& context)
{
  if(visible)
    sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

void
InvisibleBlock::hit(Player& )
{
  if(visible)
    return;

  sprite->set_action("empty");
  sound_manager->play("brick");
  start_bounce();
  flags |= FLAG_SOLID;
  visible = true;
}

//IMPLEMENT_FACTORY(InvisibleBlock, "invisible_block");
