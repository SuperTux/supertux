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

#ifndef SUPERTUX_WORLDMAP_H
#define SUPERTUX_WORLDMAP_H

#include <vector>
#include <string>

#include <SDL_mixer.h>

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

struct Tile
{
  Surface* sprite;

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

enum Direction { NONE, WEST, EAST, NORTH, SOUTH };

std::string direction_to_string(Direction d);
Direction   string_to_direction(const std::string& d);
Direction reverse_dir(Direction d);

class WorldMap;

class Tux
{
public:
  Direction back_direction;
private:
  WorldMap* worldmap;
  Surface* sprite;

  Direction input_direction;
  Direction direction;
  Point tile_pos;
  /** Length by which tux is away from its current tile, length is in
      input_direction direction */
  float offset;
  bool  moving;

  void stop();
public: 
  Tux(WorldMap* worldmap_);
  
  void draw(const Point& offset);
  void update(float delta);

  void set_direction(Direction d) { input_direction = d; }

  bool is_moving() const { return moving; }
  Point get_pos();
  Point get_tile_pos() const { return tile_pos; } 
  void  set_tile_pos(Point p) { tile_pos = p; } 
};

/** */
class WorldMap
{
private:
  Tux* tux;

  bool quit;

  Surface* level_sprite;
  Surface* leveldot_green;
  Surface* leveldot_red;

  std::string name;
  std::string music;

  std::vector<int> tilemap;
  int width;
  int height;

public:
  struct Level
  {
    int x;
    int y;
    std::string name;
    bool solved;

    // Directions which are walkable from this level
    bool north;
    bool east;
    bool south;
    bool west;
  };

private:
  typedef std::vector<Level> Levels;
  Levels levels;

  Mix_Music* song;

  Direction input_direction;
  bool enter_level;

  Point offset;
  std::string savegame_file;

  void draw_status();
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
  void draw(const Point& offset);

  Point get_next_tile(Point pos, Direction direction);
  Tile* at(Point pos);
  WorldMap::Level* at_level();

  /** Check if it is possible to walk from \a pos into \a direction,
      if possible, write the new position to \a new_pos */
  bool path_ok(Direction direction, Point pos, Point* new_pos);

  void savegame(const std::string& filename);
  void loadgame(const std::string& filename);
};

} // namespace WorldMapNS

#endif

/* Local Variables: */
/* mode:c++ */
/* End: */
