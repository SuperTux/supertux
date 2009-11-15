//  SuperTux - End Sequence: Tux walks right
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

#include <config.h>
#include "object/endsequence_walkright.hpp"
#include "supertux/sector.hpp"
#include "supertux/mainloop.hpp"
#include "object/player.hpp"

EndSequenceWalkRight::EndSequenceWalkRight()
: EndSequence()
{
}

EndSequenceWalkRight::~EndSequenceWalkRight()
{
}

void
EndSequenceWalkRight::draw(DrawingContext& /*context*/)
{
}

void
EndSequenceWalkRight::starting()
{
  EndSequence::starting();
  last_x_pos = -1;
  endsequence_timer.start(7.3f * main_loop->get_speed());
}

void
EndSequenceWalkRight::running(float elapsed_time)
{
  EndSequence::running(elapsed_time);
  Player& tux = *Sector::current()->player;

  if (tux_may_walk) {
    end_sequence_controller->press(Controller::RIGHT);
    if (int(last_x_pos) == int(tux.get_pos().x)) {
      end_sequence_controller->press(Controller::JUMP);
    }
  }

  last_x_pos = tux.get_pos().x;

  if (endsequence_timer.check()) isdone = true;
}

void
EndSequenceWalkRight::stopping()
{
  EndSequence::stopping();
}
