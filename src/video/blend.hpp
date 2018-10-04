//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_VIDEO_BLEND_HPP
#define HEADER_SUPERTUX_VIDEO_BLEND_HPP

#include "video/gl.hpp"

class Blend
{
public:
  static const Blend NONE;
  static const Blend BLEND;
  static const Blend ADD;
  static const Blend MOD;

public:
  static Blend from_string(const std::string& text);

public:
  GLenum sfactor;
  GLenum dfactor;

  Blend()
    : sfactor(GL_SRC_ALPHA), dfactor(GL_ONE_MINUS_SRC_ALPHA)
  {}

  Blend(GLenum s, GLenum d)
    : sfactor(s), dfactor(d)
  {}

  bool operator==(const Blend& other) const {
    return (sfactor == other.sfactor &&
            dfactor == other.dfactor);
  }

  std::string to_string() const;
};

#endif

/* EOF */
