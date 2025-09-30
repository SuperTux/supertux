//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#ifndef HEADER_SUPERTUX_SPRITE_SPRITE_CONFIG_HPP
#define HEADER_SUPERTUX_SPRITE_SPRITE_CONFIG_HPP

#include <string>

#include "video/blend.hpp"
#include "video/color.hpp"

class ReaderMapping;

struct SpriteConfig final
{
public:
  SpriteConfig();
  SpriteConfig(const ReaderMapping& reader);

public:
  std::string action;

  int loops;
  float angle;
  float alpha;
  Color color;
  Blend blend;
};

#endif

/* EOF */
