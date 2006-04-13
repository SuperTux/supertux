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

#include <string>
#include <stdio.h>
#include "level.hpp"
#include "game_session.hpp"
#include "flip_level_transformer.hpp"

namespace Scripting
{
  Level::Level()
  {}

  Level::~Level()
  {}

  void
  Level::finish(bool win)
  {
    GameSession::current()->finish(win);
  }

  void
  Level::spawn(const std::string& sector, const std::string& spawnpoint)
  {
    GameSession::current()->respawn(sector, spawnpoint);
  }

  void
  Level::flip_vertically()
  {
    FlipLevelTransformer flip_transformer;
    flip_transformer.transform(GameSession::current()->get_current_level());
  }
}
