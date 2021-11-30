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
  end_sequence_controller(new CodeController())
{
}

EndSequence::~EndSequence()
{
}

void
EndSequence::update(float dt_sec)
{
  if (!isrunning) return;
  running(dt_sec);
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
EndSequence::is_running() const
{
  return isrunning;
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
EndSequence::running(float /*dt_sec*/)
{
  end_sequence_controller->update();
}

void
EndSequence::stopping()
{
}

const Controller*
EndSequence::get_controller() const
{
  return end_sequence_controller.get();
}

/* EOF */
