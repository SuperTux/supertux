//  $Id: worldmap.hpp 3327 2006-04-13 15:02:40Z ravu_al_hemio $
//
//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "sprite_change.hpp"
#include "sprite/sprite_manager.hpp"
#include "sprite/sprite.hpp"
#include "video/drawing_context.hpp"

namespace WorldMapNS
{

SpriteChange::SpriteChange(const lisp::Lisp* lisp)
  : change_on_touch(false), in_stay_action(false)
{
  lisp->get("x", pos.x);
  lisp->get("y", pos.y);
  lisp->get("change-on-touch", change_on_touch);
  
  std::string spritefile = "";
  lisp->get("sprite", spritefile);
  sprite.reset(sprite_manager->create(spritefile));

  lisp->get("stay-action", stay_action);
  lisp->get("initial-stay-action", in_stay_action);
}

SpriteChange::~SpriteChange()
{
}

void
SpriteChange::draw(DrawingContext& context)
{
  if(in_stay_action && stay_action != "") {
    sprite->set_action(stay_action);
    sprite->draw(context, pos * 32 + Vector(16, 16), LAYER_OBJECTS-1);
  }
}

void
SpriteChange::update(float )
{
}

}
