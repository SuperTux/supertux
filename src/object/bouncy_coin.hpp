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

#ifndef HEADER_SUPERTUX_OBJECT_BOUNCY_COIN_HPP
#define HEADER_SUPERTUX_OBJECT_BOUNCY_COIN_HPP

#include <memory>

#include "math/vector.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/game_object.hpp"
#include "supertux/timer.hpp"
#include "video/color.hpp"

class Sprite;

class BouncyCoin : public GameObject
{
public:
  BouncyCoin(const Vector& pos, bool emerge = false,
             const std::string& sprite_path = "images/objects/coin/coin.sprite");
  ~BouncyCoin();
  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual bool do_save() {
    return false;
  }

private:
  SpritePtr sprite;
  Vector position;
  Timer timer;
  float emerge_distance;

private:
  BouncyCoin(const BouncyCoin&);
  BouncyCoin& operator=(const BouncyCoin&);
};

#endif

/* EOF */
