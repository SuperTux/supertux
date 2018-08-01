//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_MATH_RECT_HPP
#define HEADER_SUPERTUX_MATH_RECT_HPP

#include "math/size.hpp"

class Rect
{
public:
  int left;
  int top;
  int right;
  int bottom;

public:
  Rect() :
    left(0),
    top(0),
    right(0),
    bottom(0)
  {}

  Rect(int left_, int top_, int right_, int bottom_) :
    left(left_),
    top(top_),
    right(right_),
    bottom(bottom_)
  {}

  Rect(int left_, int top_, const Size& size) :
    left(left_),
    top(top_),
    right(left_ + size.width),
    bottom(top_ + size.height)
  {}

  int get_width()  const { return right - left; }
  int get_height() const { return bottom - top; }
};

#endif

/* EOF */
