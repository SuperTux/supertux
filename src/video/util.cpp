//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "video/util.hpp"

#include <algorithm>

#include "math/size.hpp"
#include "math/vector.hpp"

namespace {

inline Size
apply_pixel_aspect_ratio_pre(const Size& window_size, float pixel_aspect_ratio)
{
  if (true)
  {
    return Size(window_size.width * pixel_aspect_ratio,
                window_size.height);
  }
  else
  {
    return Size(window_size.width,
                window_size.height * pixel_aspect_ratio);
  }
}

inline void
apply_pixel_aspect_ratio_post(const Size& real_window_size, const Size& window_size, float scale,
                                   SDL_Rect& out_viewport, Vector& out_scale)
{
  Vector transform(static_cast<float>(real_window_size.width) / window_size.width,
                   static_cast<float>(real_window_size.height) / window_size.height);
  out_viewport.x *= transform.x;
  out_viewport.y *= transform.y;

  out_viewport.w *= transform.x;
  out_viewport.h *= transform.y;

  out_scale.x = scale * transform.x;
  out_scale.y = scale * transform.y;

}

inline float
calculate_scale(const Size& min_size, const Size& max_size,
                      const Size& window_size,
                      float magnification)
{
  float scale = magnification;
  if (scale == 0.0f) // magic value
  {
    scale = 1.0f;

    // Find the minimum magnification that is needed to fill the screen
    if (window_size.width > max_size.width ||
        window_size.height > max_size.height)
    {
      scale = std::max(static_cast<float>(window_size.width) / max_size.width,
                       static_cast<float>(window_size.height) / max_size.height);
    }

    // If the resulting area would violate min_size, scale it down
    if (window_size.width / scale < min_size.width ||
        window_size.height / scale < min_size.height)
    {
      scale = std::min(static_cast<float>(window_size.width) / min_size.width,
                       static_cast<float>(window_size.height) / min_size.height);
    }
  }

  return scale;
}

inline SDL_Rect
calculate_viewport(const Size& max_size, const Size& window_size, float scale)
{
  SDL_Rect viewport;

  viewport.w = std::min(window_size.width,
                            static_cast<int>(scale * max_size.width));
  viewport.h = std::min(window_size.height,
                            static_cast<int>(scale * max_size.height));

  // Center the viewport in the window
  viewport.x = std::max(0, (window_size.width - viewport.w) / 2);
  viewport.y = std::max(0, (window_size.height - viewport.h) / 2);

  return viewport;
}

} // namespace

void calculate_viewport(const Size& min_size, const Size& max_size,
                        const Size& real_window_size,
                        float pixel_aspect_ratio, float magnification,
                        Vector& out_scale,
                        Size& out_logical_size,
                        SDL_Rect& out_viewport)
{
  // Transform the real window_size by the aspect ratio, then do
  // calculations on that virtual window_size
  Size window_size = apply_pixel_aspect_ratio_pre(real_window_size, pixel_aspect_ratio);

  float scale = calculate_scale(min_size, max_size, window_size, magnification);

  // Calculate the new viewport size
  out_viewport = calculate_viewport(max_size, window_size, scale);

  out_logical_size.width = static_cast<int>(out_viewport.w / scale);
  out_logical_size.height = static_cast<int>(out_viewport.h / scale);

  // Transform the virtual window_size back into real window coordinates
  apply_pixel_aspect_ratio_post(real_window_size, window_size, scale,
                                out_viewport, out_scale);
}

float calculate_pixel_aspect_ratio(const Size& source, const Size& target)
{
  float source_aspect = 16.0f / 9.0f; // random guess
  if (source != Size(0, 0))
  {
    source_aspect =
      static_cast<float>(source.width) /
      static_cast<float>(source.height);
  }

  float target_aspect =
    static_cast<float>(target.width) /
    static_cast<float>(target.height);

  return target_aspect / source_aspect;
}

/* EOF */
