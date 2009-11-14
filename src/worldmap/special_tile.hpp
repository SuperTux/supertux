//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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
#ifndef __WORLDMAP_SPECIAL_TILE_HPP__
#define __WORLDMAP_SPECIAL_TILE_HPP__

#include <memory>
#include <string>
#include "supertux/game_object.hpp"
#include "math/vector.hpp"
#include "lisp/lisp.hpp"

class Sprite;

namespace WorldMapNS
{

class SpecialTile : public GameObject
{
public:
  SpecialTile(const lisp::Lisp* lisp);
  virtual ~SpecialTile();

  virtual void draw(DrawingContext& context);
  virtual void update(float elapsed_time);

  Vector pos;

  /** Sprite to render instead of guessing what image to draw */
  std::auto_ptr<Sprite> sprite;

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

}

#endif
