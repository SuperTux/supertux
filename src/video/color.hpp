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

#ifndef HEADER_SUPERTUX_VIDEO_COLOR_HPP
#define HEADER_SUPERTUX_VIDEO_COLOR_HPP

#include <string>
#include <vector>

#include <SDL_image.h>

class Color final
{
public:
  static const Color BLACK;
  static const Color RED;
  static const Color GREEN;
  static const Color BLUE;
  static const Color CYAN;
  static const Color MAGENTA;
  static const Color YELLOW;
  static const Color WHITE;

public:
  static Color from_rgb888(uint8_t r, uint8_t g, uint8_t b)
  {
    return Color(static_cast<float>(r) / 255.0f,
                 static_cast<float>(g) / 255.0f,
                 static_cast<float>(b) / 255.0f);
  }

  static Color from_rgba8888(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
  {
    return Color(static_cast<float>(r) / 255.0f,
                 static_cast<float>(g) / 255.0f,
                 static_cast<float>(b) / 255.0f,
                 static_cast<float>(a) / 255.0f);
  }

public:
  Color();

  Color(float red_, float green_, float blue_, float alpha_ = 1.0);

  Color(const std::vector<float>& vals);

  bool operator==(const Color& other) const;
  bool operator!=(const Color& other) const;

  float greyscale() const;

  bool operator < (const Color& other) const;

  std::vector<float> toVector();

  inline uint8_t r8() const { return static_cast<uint8_t>(255.0f * red); }
  inline uint8_t g8() const { return static_cast<uint8_t>(255.0f * green); }
  inline uint8_t b8() const { return static_cast<uint8_t>(255.0f * blue); }
  inline uint8_t a8() const { return static_cast<uint8_t>(255.0f * alpha); }

  inline uint32_t rgba() const
  {
    return ((static_cast<uint32_t>(a8()) << 24u) |
            (static_cast<uint32_t>(b8()) << 16u) |
            (static_cast<uint32_t>(g8()) <<  8u) |
            (static_cast<uint32_t>(r8()) <<  0u));
  }

  /** Return a human-readable string representation for this color */
  std::string to_string() const
  {
    return std::to_string(red) + " " + std::to_string(green) + " " + std::to_string(blue);
  }

  SDL_Color to_sdl_color() const
  {
    return { r8(), g8(), b8(), a8() };
  }

public:
  float red, green, blue, alpha;
};

#endif

/* EOF */
