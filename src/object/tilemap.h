//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SUPERTUX_TILEMAP_H
#define SUPERTUX_TILEMAP_H

#include <vector>
#include <stdint.h>

#include "special/game_object.h"
#include "serializable.h"
#include "math/vector.h"

namespace lisp {
class Lisp;
}

using namespace SuperTux;

class Level;
class TileManager;
class Tile;

/**
 * This class is reponsible for drawing the level tiles
 */
class TileMap : public GameObject, public Serializable
{
public:
  TileMap();
  TileMap(const lisp::Lisp& reader);
  TileMap(int layer_, bool solid_, size_t width_, size_t height_);
  virtual ~TileMap();

  virtual void write(lisp::Writer& writer);

  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

  void set(int width, int height, const std::vector<unsigned int>& vec,
      int layer, bool solid);

  /** resizes the tilemap to a new width and height (tries to not destroy the
   * existing map)
   */
  void resize(int newwidth, int newheight);

  /** Flip the all tile map vertically. The purpose of this is to let
      player to play the same level in a different way :) */
  void do_vertical_flip();

  size_t get_width() const
  { return width; }

  size_t get_height() const
  { return height; }

  int get_layer() const
  { return layer; }
  
  bool is_solid() const
  { return solid; }

  /// returns tile in row y and column y (of the tilemap)
  const Tile* get_tile(int x, int y) const;
  /// returns tile at position pos (in world coordinates)
  const Tile* get_tile_at(const Vector& pos) const;

  void change(int x, int y, uint32_t newtile);

  void change_at(const Vector& pos, uint32_t newtile);

private:
  std::vector<uint32_t> tiles;
  
private:
  TileManager* tilemanager;
  bool solid;
  float speed;
  int width, height;
  int layer;

  bool vertical_flip;
};

#endif /*SUPERTUX_TILEMAP_H*/

