//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "math/vector.hpp"
#include "lisp/lisp.hpp"
#include "control/controller.hpp"
#include "statistics.hpp"
#include "timer.hpp"
#include "screen.hpp"
#include "tile_manager.hpp"
#include "game_object.hpp"
#include "console.hpp"
#include "../level.hpp"

class Sprite;
class Menu;
class SpawnPoint;
class GameObject;
class TileMap;

namespace WorldMapNS {

class Tux;
class LevelTile;
class SpecialTile;
class SpriteChange;

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

/**
 * Screen that displays a worldmap
 */
class WorldMap : public Screen
{
private:
  Tux* tux;

  static WorldMap* current_;

  std::auto_ptr<Menu> worldmap_menu;

  Vector camera_offset;

  std::string name;
  std::string music;

  typedef std::vector<GameObject*> GameObjects;
  GameObjects game_objects;
  TileMap* solids;
  
  std::auto_ptr<TileManager> tile_manager;
  
public:
  /** Variables to deal with the passive map messages */
  Timer passive_message_timer;
  std::string passive_message;

private:
  std::string map_filename;
  std::string levels_path;

  typedef std::vector<SpecialTile*> SpecialTiles;
  SpecialTiles special_tiles;
  typedef std::vector<LevelTile*> LevelTiles;
  LevelTiles levels;
  typedef std::vector<SpriteChange*> SpriteChanges;
  SpriteChanges sprite_changes;
  typedef std::vector<SpawnPoint*> SpawnPoints;
  SpawnPoints spawn_points;

  Statistics total_stats;

  typedef std::vector<HSQOBJECT> ScriptList;
  ScriptList scripts;      

public:
  WorldMap(const std::string& filename);
  ~WorldMap();

  void add_object(GameObject* object);

  static WorldMap* current()
  { return current_; }

  virtual void setup();

  /** Update worldmap state */
  virtual void update(float delta);
  /** Draw worldmap */
  virtual void draw(DrawingContext& context);

  Vector get_next_tile(Vector pos, Direction direction);
  const Tile* at(Vector pos);

  size_t level_count();
  size_t solved_level_count();

  /**
   * gets called from the GameSession when a level has been successfully
   * finished
   */
  void finished_level(Level* level);

  LevelTile* at_level();
  SpecialTile* at_special_tile();
  SpriteChange* at_sprite_change(const Vector& pos);

  /** Check if it is possible to walk from \a pos into \a direction,
      if possible, write the new position to \a new_pos */
  bool path_ok(Direction direction, const Vector& pos, Vector* new_pos);

  /**
   * Save worldmap state to squirrel state table
   */
  void save_state();

  /**
   * Load worldmap state from squirrel state table
   */
  void load_state();

  const std::string& get_title() const
  { return name; }

  /**
   * runs a script in the context of the worldmap (and keeps a reference to 
   * the script (so the script gets destroyed when the worldmap is destroyed)
   */
  HSQUIRRELVM run_script(std::istream& in, const std::string& sourcename);
    
private:
  void get_level_title(LevelTile& level);
  void draw_status(DrawingContext& context);
  void calculate_total_stats();

  void load(const std::string& filename);  
  void on_escape_press();
  void parse_special_tile(const lisp::Lisp* lisp);
  void parse_sprite_change(const lisp::Lisp* lisp);
};

} // namespace WorldMapNS

#endif
