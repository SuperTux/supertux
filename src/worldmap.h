//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_WORLDMAP_HXX
#define HEADER_WORLDMAP_HXX

#include <vector>
#include <string>

namespace WorldMapNS {

struct Point
{
  Point() : x(0), y(0) {}

  Point(const Point& pos)
    : x(pos.x), y(pos.y) {}

  Point& operator=(const Point& pos)
  { x = pos.x;
    y = pos.y; 
    return *this; }

  Point(int x_, int y_)
    : x(x_), y(y_) {}

  int x;
  int y;
};

struct Level
{
  int x;
  int y;
  std::string name;
};

struct Tile
{
  texture_type sprite;

  // Directions in which Tux is allowed to walk from this tile
  bool north;
  bool east;
  bool south;
  bool west;

  /** Stop on this tile or walk over it? */
  bool stop;
};

class TileManager
{
private:
  typedef std::vector<Tile*> Tiles;
  Tiles tiles;
  static TileManager* instance_ ;

 TileManager();
public:
  static TileManager* instance() { return instance_ ? instance_ : instance_ = new TileManager(); }

  void load();
  Tile* get(int i);
};

/** */
class WorldMap
{
private:
  texture_type tux_sprite;
  texture_type level_sprite;
  bool quit;

  std::string name;
  std::string music;

  std::vector<int> tilemap;
  int width;
  int height;

  typedef std::vector<Level> Levels;
  Levels levels;

  Mix_Music* song;

  enum Direction { NONE, WEST, EAST, NORTH, SOUTH };
  Direction tux_direction;
  Point tux_tile_pos;
  /** Length by which tux is away from its current tile, length is in
      input_direction direction */
  float tux_offset;
  bool tux_moving;

  Direction input_direction;
  bool enter_level;

  Tile* at(Point pos);
  Point get_next_tile(Point pos, Direction direction);
  bool path_ok(Direction direction, Point old_pos, Point* new_pos);
public:
  WorldMap();
  ~WorldMap();

  /** Busy loop */
  void display();

  void load_map();
  
  void get_input();

  /** Update Tux position */
  void update();

  /** Draw one frame */
  void draw();
};

} // namespace WorldMapNS

void worldmap_run();

#endif

/* Local Variables: */
/* mode:c++ */
/* End */
