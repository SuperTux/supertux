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
#ifndef __TILEMAP_H__
#define __TILEMAP_H__

#include <vector>
#include "game_object.h"
#include "serializable.h"
#include "vector.h"

class Level;
class TileManager;
class LispReader;
class Tile;

/**
 * This class is reponsible for drawing the level tiles
 */
class TileMap : public GameObject, public Serializable
{
public:
  TileMap();
  TileMap(LispReader& reader);
  virtual ~TileMap();

  virtual void write(LispWriter& writer);

  virtual void action(float elapsed_time);
  virtual void draw(DrawingContext& context);

  void set(int width, int height, const std::vector<unsigned int>& vec,
      int layer, bool solid);

  /** resizes the tilemap to a new width and height (tries to not destroy the
   * existing map)
   */
  void resize(int newwidth, int newheight);

  size_t get_width() const
  { return width; }

  size_t get_height() const
  { return height; }
  
  bool is_solid() const
  { return solid; }

  unsigned int get_tile_id_at(const Vector& pos) const;

  /// returns tile in row y and column y (of the tilemap)
  Tile* get_tile(int x, int y) const;
  /// returns tile at position pos (in world coordinates)
  Tile* get_tile_at(const Vector& pos) const;

  void change(int x, int y, unsigned int newtile);

  void change_at(const Vector& pos, unsigned int newtile);

public:
  std::vector<unsigned int> tiles;
  
private:
  TileManager* tilemanager;
  bool solid;
  float speed;
  int width, height;
  int layer;
};

#endif

