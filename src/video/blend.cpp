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

#include "video/blend.hpp"

const Blend Blend::NONE(GL_ONE, GL_ZERO);
const Blend Blend::BLEND(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
const Blend Blend::ADD(GL_SRC_ALPHA, GL_ONE);
const Blend Blend::MOD(GL_DST_COLOR, GL_ZERO);

/* EOF */
