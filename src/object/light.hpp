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

#ifndef __LIGHT_HPP__
#define __LIGHT_HPP__

#include "game_object.hpp"
#include "lisp/lisp.hpp"
#include "math/vector.hpp"
#include "video/color.hpp"

class Sprite;

class Light : public GameObject
{
public:
  Light(const Vector& center, const Color& color = Color(1.0, 1.0, 1.0, 1.0));
  virtual ~Light();

  void update(float elapsed_time);
  void draw(DrawingContext& context);

protected:
  Vector position;
  Color color;
  Sprite* sprite;
};

#endif
