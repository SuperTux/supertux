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

#ifndef SUPERTUX_SCENE_H
#define SUPERTUX_SCENE_H

#include "texture.h"
#include "timer.h"

#define FRAME_RATE 10 // 100 Frames per second (10ms)

// Player stats
struct PlayerStatus
{
  int  score;
  int  distros;
  int  lives;
  enum BonusType { NO_BONUS, GROWUP_BONUS, FLOWER_BONUS };
  BonusType bonus;

  int  score_multiplier;

  PlayerStatus();

  void reset();
};

std::string bonus_to_string(PlayerStatus::BonusType b);
PlayerStatus::BonusType string_to_bonus(const std::string& str);

extern PlayerStatus player_status;

extern float scroll_x;
extern unsigned int global_frame_counter;

#endif /*SUPERTUX_SCENE_H*/
