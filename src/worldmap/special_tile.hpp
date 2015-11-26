//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmail.com>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_WORLDMAP_SPECIAL_TILE_HPP
#define HEADER_SUPERTUX_WORLDMAP_SPECIAL_TILE_HPP

#include <memory>
#include <string>

#include "math/vector.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/game_object.hpp"
#include "util/reader_fwd.hpp"

namespace worldmap {

class SpecialTile : public GameObject
{
public:
  SpecialTile(const ReaderMapping& lisp);
  virtual ~SpecialTile();

  virtual void draw(DrawingContext& context);
  virtual void update(float elapsed_time);

public:
  Vector pos;

  /** Sprite to render instead of guessing what image to draw */
  SpritePtr sprite;

  /** Message to show in the Map */
  std::string map_message;
  bool passive_message;

  /** Script to execute when tile is touched */
  std::string script;

  /** Hide special tile */
  bool invisible;

  /** Only applies actions (ie. passive messages) when going to that direction */
  bool apply_action_north;
  bool apply_action_east;
  bool apply_action_south;
  bool apply_action_west;
};

} // namespace worldmap

#endif

/* EOF */
