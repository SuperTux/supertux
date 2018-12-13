//  SuperTux
//  Copyright (C) 2004-2018 Ingo Ruhnke <grumbel@gmail.com>
//                2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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
#include "worldmap/level_tile.hpp"
#include "worldmap/sprite_change.hpp"
#include "worldmap/tux.hpp"
#include "worldmap/worldmap.hpp"

namespace worldmap {

WorldMapState::WorldMapState(WorldMap& worldmap) :
  m_worldmap(worldmap)
{
}

void
WorldMapState::load_state()
{
  log_debug << "loading worldmap state" << std::endl;

  HSQUIRRELVM vm = SquirrelVirtualMachine::current()->get_vm();
  SQInteger oldtop = sq_gettop(vm);

  try {
    // get state table
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_table_entry(vm, "worlds");
    get_table_entry(vm, m_worldmap.m_map_filename);

    // load tux
    get_table_entry(vm, "tux");

    Vector p;
    if (!get_float(vm, "x", p.x) || !get_float(vm, "y", p.y))
    {
      log_warning << "Player position not set, respawning." << std::endl;
      m_worldmap.move_to_spawnpoint("main");
    }
    std::string back_str = read_string(vm, "back");
    m_worldmap.m_tux->m_back_direction = string_to_direction(back_str);
    m_worldmap.m_tux->set_tile_pos(p);

    int tile_data = m_worldmap.tile_data_at(p);
    if (!( tile_data & ( Tile::WORLDMAP_NORTH | Tile::WORLDMAP_SOUTH | Tile::WORLDMAP_WEST | Tile::WORLDMAP_EAST ))) {
      log_warning << "Player at illegal position " << p.x << ", " << p.y << " respawning." << std::endl;
      m_worldmap.move_to_spawnpoint("main");
    }

    sq_pop(vm, 1);

    // load levels
    get_table_entry(vm, "levels");
    for (auto& level : m_worldmap.get_objects_by_type<LevelTile>()) {
      sq_pushstring(vm, level.get_level_filename().c_str(), -1);
      if (SQ_SUCCEEDED(sq_get(vm, -2))) {
        if (!get_bool(vm, "solved", level.m_solved))
        {
          level.m_solved = false;
        }
        if (!get_bool(vm, "perfect", level.m_perfect))
        {
          level.m_perfect = false;
        }
        level.update_sprite_action();
        level.m_statistics.unserialize_from_squirrel(vm);
        sq_pop(vm, 1);
      }
    }

    // leave levels table
    sq_pop(vm, 1);

    try {
      get_table_entry(vm, "tilemaps");
      sq_pushnull(vm); // Null-iterator
      while (SQ_SUCCEEDED(sq_next(vm, -2)))
      {
        const char* key; // Name of specific tilemap table
        if (SQ_SUCCEEDED(sq_getstring(vm, -2, &key)))
        {
          auto tilemap = m_worldmap.get_object_by_name<TileMap>(key);
          if (tilemap != nullptr)
          {
            sq_pushnull(vm); // null iterator (inner);
            while (SQ_SUCCEEDED(sq_next(vm, -2)))
            {
              const char* property_key;
              if (SQ_SUCCEEDED(sq_getstring(vm, -2, &property_key)))
              {
                auto propKey = std::string(property_key);
                if (propKey == "alpha")
                {
                  float alpha_value = 1.0;
                  if (SQ_SUCCEEDED(sq_getfloat(vm, -1, &alpha_value)))
                  {
                    tilemap->set_alpha(alpha_value);
                  }
                }
              }
              sq_pop(vm, 2); // Pop key/value from the stack
            }
            sq_pop(vm, 1); // Pop null iterator
          }
        }
        sq_pop(vm, 2); // Pop key value pair from stack
      }
      sq_pop(vm, 1); // Pop null
      sq_pop(vm, 1); // leave tilemaps table
    }
    catch(const SquirrelError&)
    {
      // Failed to get tilemap entry. This could indicate
      // that no savable tilemaps have been found. In any
      // case: This is not severe at all.
    }

    if (m_worldmap.get_object_count<SpriteChange>() > 0)
    {
      // load sprite change action:
      get_table_entry(vm, "sprite-changes");
      for (auto& sc : m_worldmap.get_objects_by_type<SpriteChange>())
      {
        auto key = std::to_string(int(sc.get_pos().x)) + "_" +
                   std::to_string(int(sc.get_pos().y));
        sq_pushstring(vm, key.c_str(), -1);
        if (SQ_SUCCEEDED(sq_get(vm, -2))) {
          bool show_stay_action = false;
          if (!get_bool(vm, "show-stay-action", show_stay_action))
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
          sq_pop(vm, 1);
        }
      }

      // Leave sprite changes table
      sq_pop(vm, 1);
    }

  } catch(std::exception& e) {
    log_debug << "Not loading worldmap state: " << e.what() << std::endl;
    save_state(); // make new initial save
    m_worldmap.move_to_spawnpoint("main"); // set tux to main spawnpoint
  }
  sq_settop(vm, oldtop);

  m_worldmap.m_in_level = false;
}

void
WorldMapState::save_state() const
{
  HSQUIRRELVM vm = SquirrelVirtualMachine::current()->get_vm();
  SQInteger oldtop = sq_gettop(vm);

  try {
    // get state table
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_or_create_table_entry(vm, "worlds");

    delete_table_entry(vm, m_worldmap.m_map_filename.c_str());

    // construct new table for this worldmap
    begin_table(vm, m_worldmap.m_map_filename.c_str());

    // store tux
    begin_table(vm, "tux");

    store_float(vm, "x", m_worldmap.m_tux->get_tile_pos().x);
    store_float(vm, "y", m_worldmap.m_tux->get_tile_pos().y);
    store_string(vm, "back", direction_to_string(m_worldmap.m_tux->m_back_direction));

    end_table(vm, "tux");

    // sprite change objects:
    if (m_worldmap.get_object_count<SpriteChange>() > 0)
    {
      begin_table(vm, "sprite-changes");

      for (const auto& sc : m_worldmap.get_objects_by_type<SpriteChange>())
      {
        auto key = std::to_string(int(sc.get_pos().x)) + "_" +
                   std::to_string(int(sc.get_pos().y));
        begin_table(vm, key.c_str());
        store_bool(vm, "show-stay-action", sc.show_stay_action());
        end_table(vm, key.c_str());
      }
      end_table(vm, "sprite-changes");
    }

    // tilemap visibility
    sq_pushstring(vm, "tilemaps", -1);
    sq_newtable(vm);
    for (auto& tilemap : m_worldmap.get_objects_by_type<::TileMap>())
    {
      if (!tilemap.get_name().empty())
      {
        sq_pushstring(vm, tilemap.get_name().c_str(), -1);
        sq_newtable(vm);
        store_float(vm, "alpha", tilemap.get_alpha());
        if (SQ_FAILED(sq_createslot(vm, -3)))
        {
          throw std::runtime_error("failed to create '" + m_worldmap.m_name + "' table entry");
        }
      }
    }
    if (SQ_FAILED(sq_createslot(vm, -3)))
    {
      throw std::runtime_error("failed to create '" + m_worldmap.m_name + "' table entry");
    }

    // levels...
    begin_table(vm, "levels");

    for (const auto& level : m_worldmap.get_objects_by_type<LevelTile>()) {
      begin_table(vm, level.get_level_filename().c_str());
      store_bool(vm, "solved", level.m_solved);
      store_bool(vm, "perfect", level.m_perfect);
      level.m_statistics.serialize_to_squirrel(vm);
      end_table(vm, level.get_level_filename().c_str());
    }
    end_table(vm, "levels");

    // push world into worlds table
    end_table(vm, m_worldmap.m_map_filename.c_str());
  } catch(std::exception& ) {
    sq_settop(vm, oldtop);
  }

  sq_settop(vm, oldtop);

  m_worldmap.m_savegame.save();
}

} // namespace worldmap

/* EOF */
