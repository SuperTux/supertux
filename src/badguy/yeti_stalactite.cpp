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

#include "yeti_stalactite.hpp"

static const float SHAKE_TIME = .8;

YetiStalactite::YetiStalactite(const lisp::Lisp& lisp)
  : Stalactite(lisp)
{
}

YetiStalactite::~YetiStalactite()
{
}

void
YetiStalactite::start_shaking()
{
  timer.start(SHAKE_TIME);
  state = STALACTITE_SHAKING;
}

bool
YetiStalactite::is_hanging()
{
  return state == STALACTITE_HANGING;
}

void
YetiStalactite::active_update(float elapsed_time)
{
  if(state == STALACTITE_HANGING)
    return;

  Stalactite::active_update(elapsed_time);
}

IMPLEMENT_FACTORY(YetiStalactite, "yeti_stalactite")
