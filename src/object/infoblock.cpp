//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "infoblock.hpp"
#include "game_session.hpp"
#include "resources.hpp"
#include "sprite/sprite_manager.hpp"
#include "object_factory.hpp"
#include "lisp/lisp.hpp"
#include "sector.hpp"
#include "log.hpp"

InfoBlock::InfoBlock(const lisp::Lisp& lisp)
  : Block(sprite_manager->create("images/objects/bonus_block/infoblock.sprite"))
{
  Vector pos;
  lisp.get("x", pos.x);
  lisp.get("y", pos.y);
  bbox.set_pos(pos);

  if(!lisp.get("message", message)) {
    log_warning << "No message in InfoBlock" << std::endl;
  }
  //stopped = false;
  //ringing = new AmbientSound(get_pos(), 0.5, 300, 1, "sounds/phone.wav");
  //Sector::current()->add_object(ringing);  
}

InfoBlock::~InfoBlock()
{
}

void
InfoBlock::hit(Player& )
{
  start_bounce();
  //if (!stopped) {
  //  ringing->remove_me();
  //  stopped = true;
  //}
  GameSession::current()->display_info_box(message);
}

IMPLEMENT_FACTORY(InfoBlock, "infoblock")
