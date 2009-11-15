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

#ifndef HEADER_SUPERTUX_OBJECT_SPOTLIGHT_HPP
#define HEADER_SUPERTUX_OBJECT_SPOTLIGHT_HPP

#include "lisp/lisp.hpp"
#include "math/vector.hpp"
#include "supertux/game_object.hpp"
#include "video/color.hpp"

class Sprite;

class Spotlight : public GameObject
{
public:
  Spotlight(const lisp::Lisp& reader);
  virtual ~Spotlight();

  void update(float elapsed_time);
  void draw(DrawingContext& context);

private:
  Vector  position;
  float   angle;
  Sprite* center;
  Sprite* base;
  Sprite* lights;
  Sprite* light;
  Sprite* lightcone;

  Color   color;
};

#endif

/* EOF */
