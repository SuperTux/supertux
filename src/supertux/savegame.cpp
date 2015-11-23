//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/savegame.hpp"

#include <algorithm>

#include "physfs/ifile_streambuf.hpp"
#include "scripting/scripting.hpp"
#include "scripting/serialize.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/player_status.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "util/writer.hpp"
#include "worldmap/worldmap.hpp"

namespace {
using scripting::get_table_entry;
using scripting::get_or_create_table_entry;
using scripting::get_table_keys;

std::vector<LevelState> get_level_states(HSQUIRRELVM vm)
{
  std::vector<LevelState> results;

  sq_pushnull(vm);
  while(SQ_SUCCEEDED(sq_next(vm, -2)))
  {
    //here -1 is the value and -2 is the key
    const char* result;
    if(SQ_FAILED(sq_getstring(vm, -2, &result)))
    {
      std::ostringstream msg;
      msg << "Couldn't get string value";
      throw scripting::SquirrelError(vm, msg.str());
    }
    else
    {
      LevelState level_state;
      level_state.filename = result;
      scripting::get_bool(vm, "solved", level_state.solved);
      scripting::get_bool(vm, "perfect", level_state.perfect);

      results.push_back(level_state);
    }

    // pops key and val before the next iteration
    sq_pop(vm, 2);
  }

  return results;
}

} // namespace

void
LevelsetState::store_level_state(const LevelState& in_state)
{
  auto it = std::find_if(level_states.begin(), level_states.end(),
                         [&in_state](const LevelState& state)
                         {
                           return state.filename == in_state.filename;
                         });
  if (it != level_states.end())
  {
    *it = in_state;
  }
  else
  {
    level_states.push_back(in_state);
  }
}

LevelState
LevelsetState::get_level_state(const std::string& filename) const
{
  auto it = std::find_if(level_states.begin(), level_states.end(),
                         [filename](const LevelState& state)
                         {
                           return state.filename == filename;
                         });
  if (it != level_states.end())
  {
    return *it;
  }
  else
  {
    log_info << "creating new level state for " << filename << std::endl;
    LevelState state;
    state.filename = filename;
    return state;
  }
}

Savegame::Savegame(const std::string& filename) :
  m_filename(filename),
  m_player_status(new PlayerStatus)
{
}

Savegame::~Savegame()
{
}

void
Savegame::load()
{
  if (m_filename.empty())
  {
    log_debug << "no filename set for savegame, skipping load" << std::endl;
    return;
  }

  clear_state_table();

  if(!PHYSFS_exists(m_filename.c_str()))
  {
    log_info << m_filename << ": doesn't exist, not loading state" << std::endl;
  }
  else
  {
    log_debug << "loading savegame from " << m_filename << std::endl;

    try
    {
      HSQUIRRELVM vm = scripting::global_vm;

      auto doc = ReaderDocument::parse(m_filename);
      auto root = doc.get_root();

      if(root.get_name() != "supertux-savegame")
      {
        throw std::runtime_error("file is not a supertux-savegame file");
      }
      else
      {
        auto mapping = root.get_mapping();

        int version = 1;
        mapping.get("version", version);
        if(version != 1)
        {
          throw std::runtime_error("incompatible savegame version");
        }
        else
        {
          ReaderMapping tux;
          if(!mapping.get("tux", tux))
          {
            throw std::runtime_error("No tux section in savegame");
          }
          {
            m_player_status->read(tux);
          }

          ReaderMapping state;
          if(!mapping.get("state", state))
          {
            throw std::runtime_error("No state section in savegame");
          }
          else
          {
            sq_pushroottable(vm);
            get_table_entry(vm, "state");
            scripting::load_squirrel_table(vm, -1, state);
            sq_pop(vm, 2);
          }
        }
      }
    }
    catch(const std::exception& e)
    {
      log_fatal << "Couldn't load savegame: " << e.what() << std::endl;
    }
  }
}

void
Savegame::clear_state_table()
{
  HSQUIRRELVM vm = scripting::global_vm;

  // delete existing state table, if it exists
  sq_pushroottable(vm);
  {
    // create a new empty state table
    sq_pushstring(vm, "state", -1);
    sq_newtable(vm);
    if(SQ_FAILED(sq_createslot(vm, -3)))
    {
      throw scripting::SquirrelError(vm, "Couldn't create state table");
    }
  }
  sq_pop(vm, 1);
}

void
Savegame::save()
{
  if (m_filename.empty())
  {
    log_debug << "no filename set for savegame, skipping save" << std::endl;
    return;
  }

  log_debug << "saving savegame to " << m_filename << std::endl;

  { // make sure the savegame directory exists
    std::string dirname = FileSystem::dirname(m_filename);
    if(!PHYSFS_exists(dirname.c_str()))
    {
      if(!PHYSFS_mkdir(dirname.c_str()))
      {
        std::ostringstream msg;
        msg << "Couldn't create directory for savegames '"
            << dirname << "': " <<PHYSFS_getLastError();
        throw std::runtime_error(msg.str());
      }
    }

    if(!PHYSFS_isDirectory(dirname.c_str()))
    {
      std::ostringstream msg;
      msg << "Savegame path '" << dirname << "' is not a directory";
      throw std::runtime_error(msg.str());
    }
  }

  HSQUIRRELVM vm = scripting::global_vm;

  Writer writer(m_filename);

  writer.start_list("supertux-savegame");
  writer.write("version", 1);

  using namespace worldmap;
  if(WorldMap::current() != NULL)
  {
    std::ostringstream title;
    title << WorldMap::current()->get_title();
    title << " (" << WorldMap::current()->solved_level_count()
          << "/" << WorldMap::current()->level_count() << ")";
    writer.write("title", title.str());
  }

  writer.start_list("tux");
  m_player_status->write(writer);
  writer.end_list("tux");

  writer.start_list("state");

  sq_pushroottable(vm);
  sq_pushstring(vm, "state", -1);
  if(SQ_SUCCEEDED(sq_get(vm, -2)))
  {
    scripting::save_squirrel_table(vm, -1, writer);
    sq_pop(vm, 1);
  }
  sq_pop(vm, 1);
  writer.end_list("state");

  writer.end_list("supertux-savegame");
}

std::vector<std::string>
Savegame::get_worldmaps()
{
  std::vector<std::string> worlds;

  HSQUIRRELVM vm = scripting::global_vm;
  int oldtop = sq_gettop(vm);

  try
  {
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_or_create_table_entry(vm, "worlds");
    worlds = get_table_keys(vm);
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm, oldtop);

  return worlds;
}

WorldmapState
Savegame::get_worldmap_state(const std::string& name)
{
  WorldmapState result;

  HSQUIRRELVM vm = scripting::global_vm;
  int oldtop = sq_gettop(vm);

  try
  {
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_or_create_table_entry(vm, "worlds");
    get_or_create_table_entry(vm, name);
    get_or_create_table_entry(vm, "levels");

    result.level_states = get_level_states(vm);
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm, oldtop);

  return result;
}

std::vector<std::string>
Savegame::get_levelsets()
{
  std::vector<std::string> results;

  HSQUIRRELVM vm = scripting::global_vm;
  int oldtop = sq_gettop(vm);

  try
  {
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_or_create_table_entry(vm, "levelsets");
    results = get_table_keys(vm);
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm, oldtop);

  return results;
}

LevelsetState
Savegame::get_levelset_state(const std::string& basedir)
{
  LevelsetState result;

  HSQUIRRELVM vm = scripting::global_vm;
  int oldtop = sq_gettop(vm);

  try
  {
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_or_create_table_entry(vm, "levelsets");
    get_or_create_table_entry(vm, basedir);
    get_or_create_table_entry(vm, "levels");

    result.level_states = get_level_states(vm);
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm, oldtop);

  return result;
}

void
Savegame::set_levelset_state(const std::string& basedir,
                             const std::string& level_filename,
                             bool solved)
{
  LevelsetState state = get_levelset_state(basedir);

  HSQUIRRELVM vm = scripting::global_vm;
  int oldtop = sq_gettop(vm);

  try
  {
    sq_pushroottable(vm);
    get_table_entry(vm, "state");
    get_or_create_table_entry(vm, "levelsets");
    get_or_create_table_entry(vm, basedir);
    get_or_create_table_entry(vm, "levels");
    get_or_create_table_entry(vm, level_filename);

    bool old_solved = false;
    scripting::get_bool(vm, "solved", old_solved);
    scripting::store_bool(vm, "solved", solved || old_solved);
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm, oldtop);
}

/* EOF */
