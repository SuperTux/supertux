//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
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

#include <stdlib.h>
#include "scene.h"
#include "defines.h"

PlayerStatus player_status;

PlayerStatus::PlayerStatus()
  : score(0),
    distros(0),
    lives(START_LIVES),
    bonus(NO_BONUS),
    score_multiplier(1)
{
}

void PlayerStatus::reset()
{
  score = 0;
  distros = 0;
  lives = START_LIVES;
  bonus = NO_BONUS;
  score_multiplier = 1;
}

std::string bonus_to_string(PlayerStatus::BonusType b)
{
  switch (b)
    {
    case PlayerStatus::NO_BONUS:
      return "none";
    case PlayerStatus::GROWUP_BONUS:
      return "growup";
    case PlayerStatus::FLOWER_BONUS:
      return "iceflower";
    default:
      return "none";
    }
}

PlayerStatus::BonusType string_to_bonus(const std::string& str)
{
  if (str == "none")
    return PlayerStatus::NO_BONUS;
  else if (str == "growup")
    return PlayerStatus::GROWUP_BONUS;
  else if (str == "iceflower")
    return PlayerStatus::FLOWER_BONUS;
  else
    return PlayerStatus::NO_BONUS;
}

// FIXME: Move this into a view class
float scroll_x;

unsigned int global_frame_counter;

// EOF //

