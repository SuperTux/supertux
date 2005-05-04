//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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

#include "hatch.h"
#include "game_session.h"
#include "resources.h"
#include "object_factory.h"
#include "sprite/sprite.h"
#include "sprite/sprite_manager.h"
#include "video/drawing_context.h"
#include "lisp/lisp.h"
#include "lisp/writer.h"

Hatch::Hatch(const lisp::Lisp& reader)
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  bbox.set_size(64, 64);

  reader.get("sector", target_sector);
  reader.get("spawnpoint", target_spawnpoint);

  sprite = sprite_manager->create("hatch");
}

Hatch::Hatch(int x, int y, std::string sector, std::string spawnpoint)
{
  bbox.set_pos(Vector(x, y));
  bbox.set_size(64, 64);
  target_sector = sector;
  target_spawnpoint = spawnpoint;

  sprite = sprite_manager->create("hatch");
}

Hatch::~Hatch()
{
  delete sprite;
}

void
Hatch::write(lisp::Writer& writer)
{
  writer.start_list("hatch");

  writer.write_float("x", bbox.p1.x);
  writer.write_float("y", bbox.p1.y);
  writer.write_float("width", bbox.get_width());
  writer.write_float("height", bbox.get_height());
  
  writer.write_string("sector", target_sector);
  writer.write_string("spawnpoint", target_spawnpoint);

  writer.end_list("hatch");
}

void
Hatch::action(float )
{
  //Check if hatch animation is complete
  if(sprite->check_animation()) {
    sprite->set_action("normal");
    GameSession::current()->respawn(target_sector, target_spawnpoint);
  }
}

void
Hatch::draw(DrawingContext& context)
{
  sprite->draw(context, bbox.p1, LAYER_TILES);
}

void
Hatch::event(Player& , EventType type)
{
  if(type == EVENT_ACTIVATE) {
    sprite->set_action("open", 1);
  }
}

IMPLEMENT_FACTORY(Hatch, "hatch");
