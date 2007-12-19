//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef __ELECTRIFIER_H__
#define __ELECTRIFIER_H__

#include "resources.hpp"
#include "game_object.hpp"
#include "timer.hpp"

#include "SDL.h"

//Changes all tiles with the given ID to a new one for a given amount of time, then removes itself
//Used by the Kugelblitz to electrify water - can be used for other effects, too
class Electrifier : public GameObject
{
public:
  Electrifier(Uint32 oldtile, Uint32 newtile, float seconds);
  ~Electrifier();
protected:
  virtual void update(float time);
  virtual void draw(DrawingContext& context);
private:
  Uint32 change_from;
  Uint32 change_to;
  Timer duration;
};

#endif
