//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef __ELECTRIFIER_H__
#define __ELECTRIFIER_H__

#include "resources.hpp"
#include "game_object.hpp"
#include "timer.hpp"
#include <stdint.h>

//Changes all tiles with the given ID to a new one for a given amount of time, then removes itself
//Used by the Kugelblitz to electrify water - can be used for other effects, too
class Electrifier : public GameObject
{
public:
  Electrifier(uint32_t oldtile, uint32_t newtile, float seconds);
  ~Electrifier();
protected:
  virtual void update(float time);
  virtual void draw(DrawingContext& context);
private:
  uint32_t change_from;
  uint32_t change_to;
  Timer duration;
};

#endif
