//  SuperTux - End Sequence
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "object/endsequence.hpp"

#include "object/player.hpp"
#include "supertux/sector.hpp"

EndSequence::EndSequence() :
  isrunning(false),
  isdone(false),
  tux_may_walk(true),
  end_sequence_controller(0)
{
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
EndSequence::is_tux_stopped() const
{
  return !tux_may_walk;
}

bool
EndSequence::is_done() const
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
  end_sequence_controller->update();
}

void
EndSequence::stopping()
{
}

/* EOF */
