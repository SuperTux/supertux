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
#include "scripting/squirrel_util.hpp"
#include "supertux/savegame.hpp"
#include "supertux/tile.hpp"
#include "util/log.hpp"
#include "worldmap/level.hpp"
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

  HSQUIRRELVM vm = scripting::global_vm;
  SQInteger oldtop = sq_gettop(vm);

  try {
    // get state table
    sq_pushroottable(vm);
    scripting::get_table_entry(vm, "state");
    scripting::get_table_entry(vm, "worlds");
    scripting::get_table_entry(vm, m_worldmap.m_map_filename);

    // load tux
    scripting::get_table_entry(vm, "tux");

    Vector p;
    if (!scripting::get_float(vm, "x", p.x) || !scripting::get_float(vm, "y", p.y))
    {
      log_warning << "Player position not set, respawning." << std::endl;
      m_worldmap.move_to_spawnpoint("main");
    }
    std::string back_str = scripting::read_string(vm, "back");
    m_worldmap.m_tux->back_direction = string_to_direction(back_str);
    m_worldmap.m_tux->set_tile_pos(p);

    int tile_data = m_worldmap.tile_data_at(p);
    if (!( tile_data & ( Tile::WORLDMAP_NORTH | Tile::WORLDMAP_SOUTH | Tile::WORLDMAP_WEST | Tile::WORLDMAP_EAST ))) {
      log_warning << "Player at illegal position " << p.x << ", " << p.y << " respawning." << std::endl;
      m_worldmap.move_to_spawnpoint("main");
    }

    sq_pop(vm, 1);

    // load levels
    scripting::get_table_entry(vm, "levels");
    for(const auto& level : m_worldmap.m_levels) {
      sq_pushstring(vm, level->get_name().c_str(), -1);
      if (SQ_SUCCEEDED(sq_get(vm, -2))) {
        if (!scripting::get_bool(vm, "solved", level->solved))
        {
          level->solved = false;
        }
        if (!scripting::get_bool(vm, "perfect", level->perfect))
        {
          level->perfect = false;
        }
        level->update_sprite_action();
        level->statistics.unserialize_from_squirrel(vm);
        sq_pop(vm, 1);
      }
    }

    // leave levels table
    sq_pop(vm, 1);

    try {
      scripting::get_table_entry(vm, "tilemaps");
      sq_pushnull(vm); // Null-iterator
      while(SQ_SUCCEEDED(sq_next(vm, -2)))
      {
        const char* key; // Name of specific tilemap table
        if (SQ_SUCCEEDED(sq_getstring(vm, -2, &key)))
        {
          auto tilemap = m_worldmap.get_object_by_name<TileMap>(key);
          if (tilemap != nullptr)
          {
            sq_pushnull(vm); // null iterator (inner);
            while(SQ_SUCCEEDED(sq_next(vm, -2)))
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
    catch(const scripting::SquirrelError&)
    {
      // Failed to get tilemap entry. This could indicate
      // that no savable tilemaps have been found. In any
      // case: This is not severe at all.
    }

    if (m_worldmap.m_sprite_changes.size() > 0)
    {
      // load sprite change action:
      scripting::get_table_entry(vm, "sprite-changes");
      for(const auto& sc : m_worldmap.m_sprite_changes)
      {
        auto key = std::to_string(int(sc->pos.x)) + "_" +
                   std::to_string(int(sc->pos.y));
        sq_pushstring(vm, key.c_str(), -1);
        if (SQ_SUCCEEDED(sq_get(vm, -2))) {
          bool show_stay_action = false;
          if (!scripting::get_bool(vm, "show-stay-action", show_stay_action))
          {
            sc->clear_stay_action(/* propagate = */ false);
          }
          else
          {
            if (show_stay_action)
            {
              sc->set_stay_action();
            }
            else
            {
              sc->clear_stay_action(/* propagate = */ false);
            }
          }
          sq_pop(vm, 1);
        }
      }

      // Leave sprite changes table
      sq_pop(vm, 1);
    }

    // load overall statistics
    m_worldmap.m_total_stats.unserialize_from_squirrel(vm);

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
  HSQUIRRELVM vm = scripting::global_vm;
  SQInteger oldtop = sq_gettop(vm);

  try {
    // get state table
    sq_pushroottable(vm);
    scripting::get_table_entry(vm, "state");
    scripting::get_or_create_table_entry(vm, "worlds");

    scripting::delete_table_entry(vm, m_worldmap.m_map_filename.c_str());

    // construct new table for this worldmap
    scripting::begin_table(vm, m_worldmap.m_map_filename.c_str());

    // store tux
    scripting::begin_table(vm, "tux");

    scripting::store_float(vm, "x", m_worldmap.m_tux->get_tile_pos().x);
    scripting::store_float(vm, "y", m_worldmap.m_tux->get_tile_pos().y);
    scripting::store_string(vm, "back", direction_to_string(m_worldmap.m_tux->back_direction));

    scripting::end_table(vm, "tux");

    // sprite change objects:
    if (m_worldmap.m_sprite_changes.size() > 0)
    {
      scripting::begin_table(vm, "sprite-changes");

      for(const auto& sc : m_worldmap.m_sprite_changes)
      {
        auto key = std::to_string(int(sc->pos.x)) + "_" +
                   std::to_string(int(sc->pos.y));
        scripting::begin_table(vm, key.c_str());
        scripting::store_bool(vm, "show-stay-action", sc->show_stay_action());
        scripting::end_table(vm, key.c_str());
      }
      scripting::end_table(vm, "sprite-changes");
    }

    // tilemap visibility
    sq_pushstring(vm, "tilemaps", -1);
    sq_newtable(vm);
    for(const auto& object : m_worldmap.get_objects())
    {
      auto tilemap = dynamic_cast<::TileMap*>(object.get());
      if (tilemap && !tilemap->get_name().empty())
      {
        sq_pushstring(vm, tilemap->get_name().c_str(), -1);
        sq_newtable(vm);
        scripting::store_float(vm, "alpha", tilemap->get_alpha());
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
    scripting::begin_table(vm, "levels");

    for(const auto& level : m_worldmap.m_levels) {
      scripting::begin_table(vm, level->get_name().c_str());
      scripting::store_bool(vm, "solved", level->solved);
      scripting::store_bool(vm, "perfect", level->perfect);
      level->statistics.serialize_to_squirrel(vm);
      scripting::end_table(vm, level->get_name().c_str());
    }
    scripting::end_table(vm, "levels");

    // overall statistics...
    m_worldmap.m_total_stats.serialize_to_squirrel(vm);

    // push world into worlds table
    scripting::end_table(vm, m_worldmap.m_map_filename.c_str());
  } catch(std::exception& ) {
    sq_settop(vm, oldtop);
  }

  sq_settop(vm, oldtop);

  m_worldmap.m_savegame.save();
}

} // namespace worldmap

/* EOF */
