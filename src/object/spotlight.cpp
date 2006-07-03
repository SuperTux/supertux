//  $Id: light.cpp 3327 2006-04-13 15:02:40Z ravu_al_hemio $
//
//  SuperTux
//  Copyright (C) 2006 Ingo Ruhnke <grumbel@gmx.de>
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

#include "spotlight.hpp"
#include "sprite/sprite_manager.hpp"
#include "resources.hpp"
#include "video/drawing_context.hpp"
#include "object_factory.hpp"
#include "player.hpp"
#include "sector.hpp"

Spotlight::Spotlight(const lisp::Lisp& )
{
  center    = sprite_manager->create("images/objects/spotlight/spotlight_center.sprite");
  base      = sprite_manager->create("images/objects/spotlight/spotlight_base.sprite");
  lights    = sprite_manager->create("images/objects/spotlight/spotlight_lights.sprite");
  lightcone = sprite_manager->create("images/objects/spotlight/lightcone.sprite");
}

Spotlight::~Spotlight()
{
  delete center;
  delete base;
  delete lights;
  delete lightcone;
}

void
Spotlight::update(float )
{
  // FIXME: add rotation code
}

void
Spotlight::draw(DrawingContext& context)
{
  context.push_target();
  context.set_target(DrawingContext::LIGHTMAP);
 
  Vector pos(100, 300);
  lightcone->draw(context, pos, 0);
  // rotate this one 180 degree
  lightcone->draw(context, pos, 0);
  
  context.set_target(DrawingContext::NORMAL);
  base->draw(context, pos, 0);
  center->draw(context, pos, 0);

  context.pop_target();
}

IMPLEMENT_FACTORY(Spotlight, "spotlight");
