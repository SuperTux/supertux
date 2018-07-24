//  SuperTux
//  Copyright (C) 2013 LMH <lmh.0013@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_COIN_RAIN_HPP
#define HEADER_SUPERTUX_OBJECT_COIN_RAIN_HPP

#include "math/vector.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/game_object.hpp"
#include "supertux/timer.hpp"

class CoinRain : public GameObject
{
public:
  CoinRain(const Vector& pos, bool emerge=false);
  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual bool is_saveable() const {
    return false;
  }

private:
  SpritePtr sprite;
  Vector position;
  float emerge_distance;
  Timer timer;
  int counter;
  int drop;
};

#endif

/* EOF */
