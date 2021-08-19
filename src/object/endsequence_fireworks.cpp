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

#include "object/endsequence_fireworks.hpp"

#include "object/fireworks.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"

EndSequenceFireworks::EndSequenceFireworks() :
  EndSequence(),
  endsequence_timer()
{
}

EndSequenceFireworks::~EndSequenceFireworks()
{
}

void
EndSequenceFireworks::draw(DrawingContext& /*context*/)
{
}

void
EndSequenceFireworks::starting()
{
  EndSequence::starting();
  endsequence_timer.start(9.3f * ScreenManager::current()->get_speed());
  Sector::get().add<Fireworks>();
}

void
EndSequenceFireworks::running(float dt_sec)
{
  EndSequence::running(dt_sec);
  //Player& tux = *Sector::get().player;

  if (tux_may_walk) {
    end_sequence_controller->press(Control::JUMP);
  }

  if (endsequence_timer.check()) isdone = true;
}

void
EndSequenceFireworks::stopping()
{
  EndSequence::stopping();
}

/* EOF */
