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

#ifndef HEADER_SUPERTUX_WORLDMAP_WORLDMAP_HPP
#define HEADER_SUPERTUX_WORLDMAP_WORLDMAP_HPP

#include <vector>

#include "math/vector.hpp"
#include "supertux/game_object_manager.hpp"
#include "squirrel/squirrel_environment.hpp"
#include "supertux/statistics.hpp"
#include "supertux/timer.hpp"
#include "util/currenton.hpp"
#include "worldmap/direction.hpp"
#include "worldmap/spawn_point.hpp"

class Controller;
class Level;
class PlayerStatus;
class Savegame;
class Sprite;
class SquirrelEnvironment;
class TileMap;
class TileSet;

namespace worldmap {

class Camera;
class LevelTile;
class SpecialTile;
class SpriteChange;
class Teleporter;
class Tux;

class WorldMap final : public GameObjectManager,
                       public Currenton<WorldMap>
{
public:
  friend class WorldMapParser;
  friend class WorldMapState;

  static Color level_title_color;
  static Color message_color;
  static Color teleporter_message_color;

public:
  WorldMap(const std::string& filename, Savegame& savegame, const std::string& force_spawnpoint = "");
  ~WorldMap() override;

  void finish_construction();

  void setup();
  void leave();

  void draw(DrawingContext& context);
  void update(float dt_sec);

  void process_input(const Controller& controller);

  Vector get_next_tile(const Vector& pos, const Direction& direction) const;

  /** gets a bitfield of Tile::WORLDMAP_NORTH | Tile::WORLDMAP_WEST |
      ... values, which indicates the directions Tux can move to when
      at the given position. */
  int available_directions_at(const Vector& pos) const;

  /** returns a bitfield representing the union of all
      Tile::WORLDMAP_XXX values of all solid tiles at the given
      position */
  int tile_data_at(const Vector& pos) const;

  size_t level_count() const;
  size_t solved_level_count() const;

  /** gets called from the GameSession when a level has been successfully
      finished */
  void finished_level(Level* level);

  Savegame& get_savegame() const { return m_savegame; }

  /** Get a spawnpoint by its name @param name The name of the
      spawnpoint @return spawnpoint corresponding to that name */
  SpawnPoint* get_spawnpoint_by_name(const std::string& spawnpoint_name) const
  {
    for (const auto& sp : m_spawn_points) {
      if (sp->get_name() == spawnpoint_name) {
        return sp.get();
      }
    }
    return nullptr;
  }

  LevelTile* at_level() const;
  SpecialTile* at_special_tile() const;
  SpriteChange* at_sprite_change(const Vector& pos) const;
  Teleporter* at_teleporter(const Vector& pos) const;

  /** Check if it is possible to walk from \a pos into \a direction,
      if possible, write the new position to \a new_pos */
  bool path_ok(const Direction& direction, const Vector& pos, Vector* new_pos) const;

  /** Save worldmap state to squirrel state table */
  void save_state();

  /** Load worldmap state from squirrel state table */
  void load_state();

  const std::string& get_title() const { return m_name; }

  /** switch to another worldmap.
      filename is relative to data root path */
  void change(const std::string& filename, const std::string& force_spawnpoint="");

  /** Moves Tux to the given spawnpoint
      @param spawnpoint Name of the spawnpoint to move to
      @param pan Pan the camera during to new spawnpoint
      @param main_as_default Move Tux to main spawnpoint if specified spawnpoint can't be found */
  void move_to_spawnpoint(const std::string& spawnpoint, bool pan = false, bool main_as_default = true);

  /** Mark all levels as solved or unsolved */
  void set_levels_solved(bool solved, bool perfect);

  /** Sets the name of the tilemap that should fade when worldmap is set up. */
  void set_initial_fade_tilemap(const std::string& tilemap_name, int direction)
  {
    m_initial_fade_tilemap = tilemap_name;
    m_fade_direction = direction;
  }

  /** Sets the initial spawnpoint on worldmap setup */
  void set_initial_spawnpoint(const std::string& spawnpoint_name)
  {
    m_force_spawnpoint = spawnpoint_name;

    // If spawnpoint we specified can not be found,
    // don't bother moving to the main spawnpoint.
    m_main_is_default = false;
  }

  void run_script(const std::string& script, const std::string& sourcename);

  void set_passive_message(const std::string& message, float time);

  Camera& get_camera() const { return *m_camera; }

  Vector get_tux_pos();

protected:
  virtual bool before_object_add(GameObject& object) override;
  virtual void before_object_remove(GameObject& object) override;

private:
  void draw_status(DrawingContext& context);

  void load(const std::string& filename);
  void on_escape_press();

private:
  std::unique_ptr<SquirrelEnvironment> m_squirrel_environment;
  std::unique_ptr<Camera> m_camera;

  bool m_enter_level;

  Tux* m_tux;

  Savegame& m_savegame;

  TileSet* m_tileset;

  std::string m_name;
  std::string m_init_script;

  /** Variables to deal with the passive map messages */
  Timer m_passive_message_timer;
  std::string m_passive_message;

  std::string m_map_filename;
  std::string m_levels_path;

  std::vector<std::unique_ptr<SpawnPoint> > m_spawn_points;

  std::string m_force_spawnpoint; /**< if set, spawnpoint will be forced to this value */
  bool m_main_is_default;
  std::string m_initial_fade_tilemap;
  int m_fade_direction;

  bool m_in_level;

  bool m_in_world_select;

private:
  WorldMap(const WorldMap&) = delete;
  WorldMap& operator=(const WorldMap&) = delete;
};

} // namespace worldmap

#endif

/* EOF */
