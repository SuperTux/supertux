//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2023-2024 Vankata453
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

#include "sprite/sprite_config.hpp"

#include "util/reader_mapping.hpp"

SpriteConfig::SpriteConfig() :
  action(),
  loops(-1),
  angle(0.0f),
  alpha(1.0f),
  color(1.0f, 1.0f, 1.0f, 1.0f),
  blend()
{
}

SpriteConfig::SpriteConfig(const ReaderMapping& reader) :
  SpriteConfig()
{
  reader.get("action", action);
  reader.get("loops", loops);
  reader.get("angle", angle);
  reader.get("alpha", alpha);

  std::vector<float> v_color;
  reader.get("color", v_color);
  if (!v_color.empty())
    color = Color(v_color);

  std::string blend_str;
  reader.get("blend", blend_str);
  if (!blend_str.empty())
    blend = Blend_from_string(blend_str);
}

/* EOF */
