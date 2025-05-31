//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_MATH_FWD_HPP
#define HEADER_SUPERTUX_MATH_FWD_HPP

#include <glm/fwd.hpp>

template<typename T>
using Vector_t = glm::vec<2, T, glm::qualifier::defaultp>;

using Vector = Vector_t<float>;
using Vectori = Vector_t<int>;

template<typename T>
class Rect_t;

using Rect = Rect_t<int>;
using Rectf = Rect_t<float>;

template<typename T>
class Size_t;

using Size = Size_t<int>;
using Sizef = Size_t<float>;

#endif

/* EOF */
