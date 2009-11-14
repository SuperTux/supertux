//  SuperTux - End Sequence
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "endsequence.hpp"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include "main.hpp"
#include "resources.hpp"
#include "sector.hpp"
#include "gettext.hpp"
#include "object_factory.hpp"
#include "object/player.hpp"
#include "video/drawing_context.hpp"
#include "lisp/list_iterator.hpp"
#include "log.hpp"
#include "scripting/level_time.hpp"
#include "scripting/squirrel_util.hpp"

EndSequence::EndSequence()
: isrunning(false), isdone(false), tux_may_walk(true)
{
  end_sequence_controller = 0;
}

EndSequence::~EndSequence()
{
  delete end_sequence_controller;
}

void
EndSequence::update(float elapsed_time)
{
  if (!isrunning) return;
  running(elapsed_time);
}

void
EndSequence::draw(DrawingContext& /*context*/)
{
}

void
EndSequence::start()
{
  if (isrunning) return;
  isrunning = true;
  isdone = false;

  Player& tux = *Sector::current()->player;
  end_sequence_controller = new CodeController();
  tux.set_controller(end_sequence_controller);
  tux.set_speedlimit(230); //MAX_WALK_XM

  starting();
}

void
EndSequence::stop_tux()
{
  tux_may_walk = false;
}

void
EndSequence::stop()
{
  if (!isrunning) return;
  isrunning = false;
  isdone = true;
  stopping();
}

bool
EndSequence::is_tux_stopped()
{
  return !tux_may_walk;
}

 bool
EndSequence::is_done()
{
  return isdone;
}

void
EndSequence::starting()
{
}

void
EndSequence::running(float /*elapsed_time*/)
{
}

void
EndSequence::stopping()
{
}
