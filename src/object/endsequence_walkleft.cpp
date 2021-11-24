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

#include "object/endsequence_walkleft.hpp"

#include "object/player.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"

EndSequenceWalkLeft::EndSequenceWalkLeft() :
  EndSequence(),
  last_x_pos(),
  endsequence_timer()
{
}

EndSequenceWalkLeft::~EndSequenceWalkLeft()
{
}

void
EndSequenceWalkLeft::draw(DrawingContext& /*context*/)
{
}

void
EndSequenceWalkLeft::starting()
{
  EndSequence::starting();
  last_x_pos = -1;
  endsequence_timer.start(7.3f * ScreenManager::current()->get_speed());
}

void
EndSequenceWalkLeft::running(float dt_sec)
{
  EndSequence::running(dt_sec);
  Player& tux = *Sector::get().get_players()[0]; // FIXME: Do this with all players

  if (tux_may_walk) {
    end_sequence_controller->press(Control::LEFT);
    if (int(last_x_pos) == int(tux.get_pos().x)) {
      end_sequence_controller->press(Control::JUMP);
    }
  }

  last_x_pos = tux.get_pos().x;

  if (endsequence_timer.check()) isdone = true;
}

void
EndSequenceWalkLeft::stopping()
{
  EndSequence::stopping();
}

/* EOF */
