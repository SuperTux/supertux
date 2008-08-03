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

#ifndef NOSOUND
#include "musicref.h"
#endif

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

// For one way tiles
enum {
  BOTH_WAYS,
  NORTH_SOUTH_WAY,
  SOUTH_NORTH_WAY,
  EAST_WEST_WAY,
  WEST_EAST_WAY
  };

class Tile
{
public:
  Tile();
  ~Tile();
  
  Surface* sprite;

  // Directions in which Tux is allowed to walk from this tile
  bool north;
  bool east;
  bool south;
  bool west;

  /** One way tile */
  int one_way;

  /** Stop on this tile or walk over it? */
  bool stop;

  /** When set automatically turn directions when walked over such a
      tile (ie. walk smoothly a curve) */
  bool auto_walk;
};

class TileManager
{
private:
  typedef std::vector<Tile*> Tiles;
  Tiles tiles;

public:
  TileManager();
  ~TileManager();

  Tile* get(int i);
};

enum Direction { D_NONE, D_WEST, D_EAST, D_NORTH, D_SOUTH };

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
  Surface* largetux_sprite;
  Surface* firetux_sprite;
  Surface* smalltux_sprite;

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
  ~Tux();
  
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
  Surface* leveldot_teleporter;

  std::string name;
  std::string music;

  std::vector<int> tilemap;
  int width;
  int height;
  
  int start_x;
  int start_y;

  TileManager* tile_manager;

public:
  struct Level
  {
    int x;
    int y;
    std::string name;
    std::string title;
    bool solved;

    /** Filename of the extro text to show once the level is
        successfully completed */
    std::string extro_filename;

    /** Message to show in the Map during a certain time */
    std::string display_map_message;
    bool passive_message;
	 
	 /** Teleporters */
	 int teleport_dest_x;
	 int teleport_dest_y;
	 std::string teleport_message;
	 bool invisible_teleporter;

    /** If false, disables the auto walking after finishing a level */
    bool auto_path;

    /** Only applies actions (ie. map messages) when going to that direction */
    bool apply_action_north;
    bool apply_action_east;
    bool apply_action_south;
    bool apply_action_west;

    // Directions which are walkable from this level
    bool north;
    bool east;
    bool south;
    bool west;
  };

  /** Variables to deal with the passive map messages */
  Timer passive_message_timer;
  std::string passive_message;

private:
  typedef std::vector<Level> Levels;
  Levels levels;

#ifndef NOSOUND
  MusicRef song;
#endif

  Direction input_direction;
  bool enter_level;

  Point offset;
  std::string savegame_file;
  std::string map_file;

  void get_level_title(Levels::pointer level);

  void draw_status();
public:
  WorldMap();
  ~WorldMap();

  void set_map_file(std::string mapfile);

  /** Busy loop */
  void display();

  void load_map();
  
  void get_input();

  /** Update Tux position */
  void update(float delta);

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
  void loadmap(const std::string& filename);

  const std::string& get_world_title() const
    { return name; }
  
  const int& get_start_x() const
    { return start_x; }
  
  const int& get_start_y() const
    { return start_y; }

  /** This functions should be call by contrib menu to set
     all levels as played, since their state is not saved. */
  void set_levels_as_solved()
    { for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
        i->solved = true;  }

private:
  void on_escape_press();
};

} // namespace WorldMapNS

#endif

/* Local Variables: */
/* mode:c++ */
/* End: */
