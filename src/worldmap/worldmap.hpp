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
#ifndef SUPERTUX_WORLDMAP_H
#define SUPERTUX_WORLDMAP_H

#include <vector>
#include <string>

#include "math/vector.hpp"
#include "lisp/lisp.hpp"
#include "control/controller.hpp"
#include "supertux/statistics.hpp"
#include "supertux/timer.hpp"
#include "supertux/screen.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/game_object.hpp"
#include "supertux/console.hpp"
#include "supertux/level.hpp"
#include "worldmap/special_tile.hpp"
#include "worldmap/sprite_change.hpp"
#include "worldmap/teleporter.hpp"
#include "worldmap/spawn_point.hpp"
#include "worldmap/direction.hpp"

class Sprite;
class Menu;
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

std::string direction_to_string(Direction d);
Direction   string_to_direction(const std::string& d);
Direction reverse_dir(Direction d);

/**
 * Screen that runs a WorldMap, which lets the player choose a Level.
 */
class WorldMap : public Screen
{
  static Color level_title_color;
  static Color message_color;
  static Color teleporter_message_color;
private:
  Tux* tux;

  TileSet *tileset;
  bool     free_tileset;

  static WorldMap* current_;

  std::auto_ptr<Menu> worldmap_menu;

  Vector camera_offset;

  std::string name;
  std::string music;
  std::string init_script;

  typedef std::vector<GameObject*> GameObjects;
  GameObjects game_objects;
  std::list<TileMap*> solid_tilemaps;

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
  std::vector<Teleporter*> teleporters;

  Statistics total_stats;

  HSQOBJECT worldmap_table;
  typedef std::vector<HSQOBJECT> ScriptList;
  ScriptList scripts;

  Color ambient_light;
  std::string force_spawnpoint; /**< if set, spawnpoint will be forced to this value */

  bool in_level;

  /* variables to track panning to a spawn point */
  Vector pan_pos;
  bool panning;

public:
  WorldMap(const std::string& filename, const std::string& force_spawnpoint = "");
  ~WorldMap();

  void add_object(GameObject* object);
  
  void try_expose(GameObject* object);
  void try_unexpose(GameObject* object);

  static WorldMap* current()
  { return current_; }

  virtual void setup();
  virtual void leave();

  /** Update worldmap state */
  virtual void update(float delta);
  /** Draw worldmap */
  virtual void draw(DrawingContext& context);

  Vector get_next_tile(Vector pos, Direction direction);

  /**
   * gets a bitfield of Tile::WORLDMAP_NORTH | Tile::WORLDMAP_WEST | ... values, 
   * which indicates the directions Tux can move to when at the given position.
   */
  int available_directions_at(Vector pos);

  /**
   * returns a bitfield representing the union of all Tile::WORLDMAP_XXX values 
   * of all solid tiles at the given position
   */
  int tile_data_at(Vector pos);

  size_t level_count();
  size_t solved_level_count();

  /**
   * gets called from the GameSession when a level has been successfully
   * finished
   */
  void finished_level(Level* level);

  /** returns current Tux incarnation */
  Tux* get_tux() { return tux; }

  LevelTile* at_level();
  SpecialTile* at_special_tile();
  SpriteChange* at_sprite_change(const Vector& pos);
  Teleporter* at_teleporter(const Vector& pos);

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

  /**
   * switch to another worldmap.
   * filename is relative to data root path
   */
  void change(const std::string& filename, const std::string& force_spawnpoint="");

  /**
   * moves Tux to the given spawnpoint
   */
  void move_to_spawnpoint(const std::string& spawnpoint, bool pan =false);

  /**
   * returns the width (in tiles) of a worldmap
   */
  float get_width() const;

  /**
   * returns the height (in tiles) of a worldmap
   */
  float get_height() const;

private:
  void get_level_title(LevelTile& level);
  void draw_status(DrawingContext& context);
  void calculate_total_stats();

  void load(const std::string& filename);
  void on_escape_press();

  Vector get_camera_pos_for_tux();
  void clamp_camera_position(Vector& c);
};

} // namespace WorldMapNS

#endif
