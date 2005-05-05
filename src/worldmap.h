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

#include "math/vector.h"
#include "audio/musicref.h"
#include "video/screen.h"
#include "lisp/lisp.h"
#include "control/controller.h"
#include "statistics.h"
#include "timer.h"
#include "tile_manager.h"

class Sprite;
class Menu;
extern Menu* worldmap_menu;

namespace WorldMapNS {

enum WorldMapMenuIDs {
  MNID_RETURNWORLDMAP,
  MNID_QUITWORLDMAP
};

// For one way tiles
enum {
  BOTH_WAYS,
  NORTH_SOUTH_WAY,
  SOUTH_NORTH_WAY,
  EAST_WEST_WAY,
  WEST_EAST_WAY
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
  Sprite* tux_sprite;
  Controller* controller;

  Direction input_direction;
  Direction direction;
  Vector tile_pos;
  /** Length by which tux is away from its current tile, length is in
      input_direction direction */
  float offset;
  bool  moving;

  void stop();
public: 
  Tux(WorldMap* worldmap_);
  ~Tux();
  
  void draw(DrawingContext& context);
  void action(float elapsed_time);

  void set_direction(Direction dir);

  bool is_moving() const { return moving; }
  Vector get_pos();
  Vector get_tile_pos() const { return tile_pos; } 
  void  set_tile_pos(Vector p) { tile_pos = p; } 
};

/** */
class WorldMap
{
private:
  Tux* tux;

  bool quit;

  Surface* leveldot_green;
  Surface* leveldot_red;
  Surface* messagedot;
  Surface* teleporterdot;

  std::string name;
  std::string music;

  std::vector<int> tilemap;
  int width;
  int height;
  
  int start_x;
  int start_y;

  TileManager* tile_manager;

public:
  struct SpecialTile
  {
    Vector pos;

    /** Optional flags: */

    /** Position to swap to player */
    Vector teleport_dest;

    /** Message to show in the Map */
    std::string map_message;
    bool passive_message;

    /** Hide special tile */
    bool invisible;

    /** Only applies actions (ie. passive messages) when going to that direction */
    bool apply_action_north;
    bool apply_action_east;
    bool apply_action_south;
    bool apply_action_west;
  };

  struct Level
  {
    Vector pos;

    std::string name;
    std::string title;
    bool solved;

    /** Statistics for level tiles */
    Statistics statistics;

    /** Optional flags: */

    /** Check if this level should be vertically flipped */
    bool vertical_flip;

    /** Filename of the extro text to show once the level is
        successfully completed */
    std::string extro_filename;

    /** Go to this world */
    std::string next_worldmap;

    /** Quit the worldmap */
    bool quit_worldmap;

    /** If false, disables the auto walking after finishing a level */
    bool auto_path;

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
  std::string map_filename;
  std::string levels_path;

  typedef std::vector<SpecialTile> SpecialTiles;
  SpecialTiles special_tiles;

  typedef std::vector<Level> Levels;
  Levels levels;

  MusicRef song;

  Vector offset;
  std::string savegame_file;
  
  std::string intro_filename;
  bool intro_displayed;

  void get_level_title(Level& level);

  void draw_status(DrawingContext& context);

  // to avoid calculating total stats all the time. This way only
  // when need, it is calculated.
  Statistics total_stats;
  void calculate_total_stats();

public:
  WorldMap();
  ~WorldMap();

  /** Busy loop */
  void display();

  void load_map();
  
  void get_input();

  /** Update Tux position */
  void update(float delta);

  /** Draw one frame */
  void draw(DrawingContext& context);

  Vector get_next_tile(Vector pos, Direction direction);
  const Tile* at(Vector pos);

  WorldMap::Level* at_level();
  WorldMap::SpecialTile* at_special_tile();

  /** Check if it is possible to walk from \a pos into \a direction,
      if possible, write the new position to \a new_pos */
  bool path_ok(Direction direction, Vector pos, Vector* new_pos);

  /* Save map to slot */
  void savegame(const std::string& filename);
  /* Load map from slot
     You should call set_map_filename() before this */
  void loadgame(const std::string& filename);
  /* Load map directly from file */
  void loadmap(const std::string& filename);

  const std::string& get_world_title() const
    { return name; }
    
  const int& get_start_x() const
    { return start_x; }
  
  const int& get_start_y() const
    { return start_y; }

  void set_map_filename(std::string filename)
    { map_filename = filename; }

private:
  void on_escape_press();
  void parse_special_tiles(const lisp::Lisp* lisp);
};

} // namespace WorldMapNS

#endif

/* Local Variables: */
/* mode:c++ */
/* End: */
