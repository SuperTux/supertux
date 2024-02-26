//  SuperTux
//  Copyright (C) 2004-2018 Ingo Ruhnke <grumbel@gmail.com>
//                2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//                2023 Vankata453
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

#include "worldmap/worldmap_state.hpp"

#include "math/vector.hpp"
#include "object/tilemap.hpp"
#include "squirrel/squirrel_util.hpp"
#include "supertux/savegame.hpp"
#include "supertux/tile.hpp"
#include "util/log.hpp"
#include "worldmap/direction.hpp"
#include "worldmap/level_tile.hpp"
#include "worldmap/sprite_change.hpp"
#include "worldmap/tux.hpp"
#include "worldmap/worldmap.hpp"

namespace worldmap {

WorldMapState::WorldMapState(WorldMap& worldmap) :
  m_worldmap(worldmap),
  m_position_was_reset(false)
{
}


void
WorldMapState::load_state()
{
  log_debug << "loading worldmap state" << std::endl;

  WORLDMAP_STATE_SQUIRREL_VM_GUARD;
  SQInteger oldtop = sq_gettop(vm.get_vm());

  try {
    /** Get state table. **/
    sq_pushroottable(vm.get_vm());
    vm.get_table_entry("state");

    /** Get state table for all worldmaps. **/
    vm.get_table_entry("worlds");

    // If a non-canonical entry is present, replace it with a canonical one.
    const std::string old_map_filename = m_worldmap.m_map_filename.substr(1);
    if (vm.has_property(old_map_filename.c_str()))
    {
      vm.rename_table_entry(old_map_filename.c_str(), m_worldmap.m_map_filename.c_str());
    }

    /** Get state table for the current worldmap. **/
    vm.get_table_entry(m_worldmap.m_map_filename);

    // Load the current sector.
    if (vm.has_property("sector")) // Load the current sector, only if a "sector" property exists.
    {
      const std::string sector_name = vm.read_string("sector");
      if (!m_worldmap.m_sector) // If the worldmap doesn't have a current sector, try setting the new sector.
        m_worldmap.set_sector(sector_name, "", false);

      WORLDMAP_STATE_SECTOR_GUARD;

      /** Get state table for the current sector. **/
      vm.get_table_entry(sector.get_name().c_str());
    }
    else // Sector property does not exist, which may indicate outdated save file.
    {
      if (!m_worldmap.m_sector) // If the worldmap doesn't have a current sector, try setting the main one.
        m_worldmap.set_sector("main", "", false);
    }
    if (!m_worldmap.m_sector)
    {
      // Quit loading worldmap state, if there is still no current sector loaded.
      throw std::runtime_error("No sector set.");
    }

    /** Load objects. **/
    load_tux();
    load_levels();
    load_tilemap_visibility();
    load_sprite_change_objects();
  }
  catch (std::exception& err)
  {
    log_warning << "Not loading worldmap state: " << err.what() << std::endl;

    // Set default properties.
    if (!m_worldmap.m_sector)
      m_worldmap.set_sector("main", "", false); // If no current sector is present, set it to "main", or the default one.

    // Create a new initial save.
    save_state();
  }
  sq_settop(vm.get_vm(), oldtop);

  m_worldmap.m_in_level = false;
}


/** Load Tux **/
void
WorldMapState::load_tux()
{
  WORLDMAP_STATE_SQUIRREL_VM_GUARD;
  WORLDMAP_STATE_SECTOR_GUARD;

  vm.get_table_entry("tux");
  Vector p(0.0f, 0.0f);
  if (!vm.get_float("x", p.x) || !vm.get_float("y", p.y))
  {
    log_warning << "Player position not set, respawning." << std::endl;
    sector.move_to_spawnpoint("main");
    m_position_was_reset = true;
  }
  std::string back_str = vm.read_string("back");
  sector.m_tux->m_back_direction = string_to_direction(back_str);
  sector.m_tux->set_tile_pos(p);

  int tile_data = sector.tile_data_at(p);
  if (!( tile_data & ( Tile::WORLDMAP_NORTH | Tile::WORLDMAP_SOUTH | Tile::WORLDMAP_WEST | Tile::WORLDMAP_EAST ))) {
    log_warning << "Player at illegal position " << p.x << ", " << p.y << " respawning." << std::endl;
    sector.move_to_spawnpoint("main");
    m_position_was_reset = true;
  }
  sq_pop(vm.get_vm(), 1);
}

/** Load levels **/
void
WorldMapState::load_levels()
{
  WORLDMAP_STATE_SQUIRREL_VM_GUARD;
  WORLDMAP_STATE_SECTOR_GUARD;

  vm.get_or_create_table_entry("levels");
  for (auto& level : sector.get_objects_by_type<LevelTile>()) {
    sq_pushstring(vm.get_vm(), level.get_level_filename().c_str(), -1);
    if (SQ_SUCCEEDED(sq_get(vm.get_vm(), -2)))
    {
      bool solved = false;
      vm.get_bool("solved", solved);
      level.set_solved(solved);

      bool perfect = false;
      vm.get_bool("perfect", perfect);
      level.set_perfect(perfect);

      level.update_sprite_action();
      level.get_statistics().unserialize_from_squirrel(vm);
      sq_pop(vm.get_vm(), 1);
    }
  }
  sq_pop(vm.get_vm(), 1);
}

/** Load tilemap visibility **/
void
WorldMapState::load_tilemap_visibility()
{
  WORLDMAP_STATE_SQUIRREL_VM_GUARD;
  WORLDMAP_STATE_SECTOR_GUARD;

  try
  {
    if (!m_position_was_reset)
    {
      vm.get_table_entry("tilemaps");
      sq_pushnull(vm.get_vm()); // Null-iterator
      while (SQ_SUCCEEDED(sq_next(vm.get_vm(), -2)))
      {
        const char* key; // Name of specific tilemap table
        if (SQ_SUCCEEDED(sq_getstring(vm.get_vm(), -2, &key)))
        {
          auto tilemap = sector.get_object_by_name<TileMap>(key);
          if (tilemap != nullptr)
          {
            sq_pushnull(vm.get_vm()); // null iterator (inner);
            while (SQ_SUCCEEDED(sq_next(vm.get_vm(), -2)))
            {
              const char* property_key;
              if (SQ_SUCCEEDED(sq_getstring(vm.get_vm(), -2, &property_key)))
              {
                auto propKey = std::string(property_key);
                if (propKey == "alpha")
                {
                  float alpha_value = 1.0;
                  if (SQ_SUCCEEDED(sq_getfloat(vm.get_vm(), -1, &alpha_value)))
                  {
                    tilemap->set_alpha(alpha_value);
                  }
                }
              }
              sq_pop(vm.get_vm(), 2); // Pop key/value from the stack
            }
            sq_pop(vm.get_vm(), 1); // Pop null iterator
          }
        }
        sq_pop(vm.get_vm(), 2); // Pop key value pair from stack
      }
      sq_pop(vm.get_vm(), 1); // Pop null
      sq_pop(vm.get_vm(), 1); // leave tilemaps table
    }
  }
  catch(const SquirrelError&)
  {
    // Failed to get tilemap entry. This could indicate
    // that no savable tilemaps have been found. In any
    // case: This is not severe at all.
  }
}

/** Load sprite change objects **/
void
WorldMapState::load_sprite_change_objects()
{
  WORLDMAP_STATE_SQUIRREL_VM_GUARD;
  WORLDMAP_STATE_SECTOR_GUARD;

  if (sector.get_object_count<SpriteChange>() > 0)
  {
    vm.get_table_entry("sprite-changes");
    for (auto& sc : sector.get_objects_by_type<SpriteChange>())
    {
      auto key = std::to_string(int(sc.get_pos().x)) + "_" +
                 std::to_string(int(sc.get_pos().y));
      sq_pushstring(vm.get_vm(), key.c_str(), -1);
      if (SQ_SUCCEEDED(sq_get(vm.get_vm(), -2))) {
        bool show_stay_action = false;
        if (!vm.get_bool("show-stay-action", show_stay_action))
        {
          sc.clear_stay_action(/* propagate = */ false);
        }
        else
        {
          if (show_stay_action)
          {
            sc.set_stay_action();
          }
          else
          {
            sc.clear_stay_action(/* propagate = */ false);
          }
        }
        sq_pop(vm.get_vm(), 1);
      }
    }
    sq_pop(vm.get_vm(), 1); // Leave sprite change objects table.
  }
}


void
WorldMapState::save_state() const
{
  WorldMapSector& sector = m_worldmap.get_sector();

  SquirrelVM& vm = SquirrelVirtualMachine::current()->get_vm();
  SQInteger oldtop = sq_gettop(vm.get_vm());

  try {
    /** Get state table. **/
    sq_pushroottable(vm.get_vm());
    vm.get_table_entry("state");

    /** Get or create state table for all worldmaps. **/
    vm.get_or_create_table_entry("worlds");

    /** Get or create state table for the current worldmap. **/
    vm.get_or_create_table_entry(m_worldmap.m_map_filename);

    // Save the current sector.
    vm.store_string("sector", sector.get_name());

    /** Delete the table entry for the current sector and construct a new one. **/
    vm.delete_table_entry(sector.get_name().c_str());
    vm.begin_table(sector.get_name().c_str());

    /** Save objects. **/
    save_tux();
    save_levels();
    save_tilemap_visibility();
    save_sprite_change_objects();

    /** Push the current sector into the current worldmap table. **/
    vm.end_table(sector.get_name().c_str());
  }
  catch (std::exception& err)
  {
    log_warning << "Failed to save worldmap state: " << err.what() << std::endl;

    sq_settop(vm.get_vm(), oldtop);
  }

  sq_settop(vm.get_vm(), oldtop);

  m_worldmap.m_savegame.save();
}


/** Save Tux **/
void
WorldMapState::save_tux() const
{
  WORLDMAP_STATE_SQUIRREL_VM_GUARD;
  WORLDMAP_STATE_SECTOR_GUARD;

  vm.begin_table("tux");
  vm.store_float("x", sector.m_tux->get_tile_pos().x);
  vm.store_float("y", sector.m_tux->get_tile_pos().y);
  vm.store_string("back", direction_to_string(sector.m_tux->m_back_direction));
  vm.end_table("tux");
}

/** Save levels **/
void
WorldMapState::save_levels() const
{
  WORLDMAP_STATE_SQUIRREL_VM_GUARD;
  WORLDMAP_STATE_SECTOR_GUARD;

  vm.begin_table("levels");
  for (const auto& level : sector.get_objects_by_type<LevelTile>())
  {
    vm.begin_table(level.get_level_filename().c_str());

    vm.store_bool("solved", level.is_solved());
    vm.store_bool("perfect", level.is_perfect());

    level.get_statistics().serialize_to_squirrel(vm);
    vm.end_table(level.get_level_filename().c_str());
  }
  vm.end_table("levels");
}

/** Save tilemap visibility **/
void
WorldMapState::save_tilemap_visibility() const
{
  WORLDMAP_STATE_SQUIRREL_VM_GUARD;
  WORLDMAP_STATE_SECTOR_GUARD;

  sq_pushstring(vm.get_vm(), "tilemaps", -1);
  sq_newtable(vm.get_vm());
  for (auto& tilemap : sector.get_objects_by_type<::TileMap>())
  {
    if (!tilemap.get_name().empty())
    {
      sq_pushstring(vm.get_vm(), tilemap.get_name().c_str(), -1);
      sq_newtable(vm.get_vm());
      vm.store_float("alpha", tilemap.get_alpha());
      if (SQ_FAILED(sq_createslot(vm.get_vm(), -3)))
      {
        throw std::runtime_error("failed to create '" + m_worldmap.m_name + "' table entry");
      }
    }
  }
  if (SQ_FAILED(sq_createslot(vm.get_vm(), -3)))
  {
    throw std::runtime_error("failed to create '" + m_worldmap.m_name + "' table entry");
  }
}

/** Save sprite change objects **/
void
WorldMapState::save_sprite_change_objects() const
{
  WORLDMAP_STATE_SQUIRREL_VM_GUARD;
  WORLDMAP_STATE_SECTOR_GUARD;

  if (sector.get_object_count<SpriteChange>() > 0)
  {
    vm.begin_table("sprite-changes");
    for (const auto& sc : sector.get_objects_by_type<SpriteChange>())
    {
      auto key = std::to_string(int(sc.get_pos().x)) + "_" +
                 std::to_string(int(sc.get_pos().y));
      vm.begin_table(key.c_str());
      vm.store_bool("show-stay-action", sc.show_stay_action());
      vm.end_table(key.c_str());
    }
    vm.end_table("sprite-changes");
  }
}

} // namespace worldmap

/* EOF */
