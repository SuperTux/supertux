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
#ifndef SUPERTUX_PLAYERSTATUS_H
#define SUPERTUX_PLAYERSTATUS_H

#include "lisp/lisp.h"
#include "timer.h"
#include "serializable.h"

enum BonusType {
  NO_BONUS, GROWUP_BONUS, FIRE_BONUS, ICE_BONUS
};
class DrawingContext;

/** 
 * This class memorizes player status between different game sessions (for
 * example when switching maps in the worldmap)
 */
class PlayerStatus : public Serializable
{
public:
  PlayerStatus();
  void reset();     
  void incLives();
  void incCoins();

  void write(lisp::Writer& writer);
  void read(const lisp::Lisp& lisp);

  void draw(DrawingContext& context);

  int  coins;
  int  lives;
  BonusType bonus;

  int score_multiplier;
  int max_score_multiplier;
};

// global player state
extern PlayerStatus player_status;

#endif
