//  SuperTux
//  Copyright (C) 2013 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SUPERTUX_VIDEO_UTIL_HPP
#define HEADER_SUPERTUX_VIDEO_UTIL_HPP

#include "SDL_rect.h"

class Size;
class Vector;

void calculate_viewport(const Size& min_size, const Size& max_size,
                        const Size& real_window_size,
                        float pixel_aspect_ratio, float magnification,
                        Vector& out_scale,
                        Size& out_logical_size,
                        SDL_Rect& out_viewport);

float calculate_pixel_aspect_ratio(const Size& source, const Size& target);

#endif

/* EOF */
