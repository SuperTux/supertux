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
#ifndef __SCREENFADE_HPP__
#define __SCREENFADE_HPP__

#include "screen.hpp"

/**
 * Screen to be displayed simultaneously with another Screen. 
 *
 * This is used for Screen transition effects like a fade-out or a shrink-fade
 */
class ScreenFade : public Screen
{
public:
  virtual ~ScreenFade()
  {}

  /// returns true if the effect is completed
  virtual bool done() = 0;
};

#endif
