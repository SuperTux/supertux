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

#include "infoblock.h"
#include "game_session.h"
#include "resources.h"
#include "sprite/sprite_manager.h"
#include "object_factory.h"
#include "lisp/lisp.h"
#include "audio/sound_manager.h"
#include "sector.h"
#include "player.h"

InfoBlock::InfoBlock(const lisp::Lisp& lisp)
  : Block(sprite_manager->create("infoblock"))
{
  Vector pos;
  lisp.get("x", pos.x);
  lisp.get("y", pos.y);
  bbox.set_pos(pos);

  if(!lisp.get("message", message)) {
    std::cerr << "No message in InfoBlock!\n";
  }
  ringing = false;
}

InfoBlock::~InfoBlock()
{
}

void
InfoBlock::update(float elapsed_time)
{
  elapsed_time = 0;
  if (ringing) sound_manager->play_sound("phone",get_pos(),Sector::current()->player->get_pos());
}

void
InfoBlock::hit(Player& )
{
  GameSession::current()->display_info_box(message);
  ringing = false;
  start_bounce();
}

IMPLEMENT_FACTORY(InfoBlock, "infoblock")
