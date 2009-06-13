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
  void
  Level_finish(bool win)
  {
    if(GameSession::current() == NULL)
      return;

    GameSession::current()->finish(win);
  }

  void
  Level_spawn(const std::string& sector, const std::string& spawnpoint)
  {
    if(GameSession::current() == NULL)
      return;

    GameSession::current()->respawn(sector, spawnpoint);
  }

  void
  Level_flip_vertically()
  {
    FlipLevelTransformer flip_transformer;
    flip_transformer.transform(GameSession::current()->get_current_level());
  }

  void
  Level_toggle_pause()
  {
    if(GameSession::current() == NULL)
      return;
    GameSession::current()->toggle_pause();
  }

  void
  Level_edit(bool edit_mode)
  {
    if(GameSession::current() == NULL) return;
    GameSession::current()->set_editmode(edit_mode);
  }

}
