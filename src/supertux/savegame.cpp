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
#include <physfs.h>

#include "control/input_manager.hpp"
#include "physfs/physfs_file_system.hpp"
#include "physfs/util.hpp"
#include "squirrel/serialize.hpp"
#include "squirrel/squirrel_util.hpp"
#include "squirrel/squirrel_virtual_machine.hpp"
#include "supertux/player_status.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "worldmap/worldmap.hpp"

namespace {

std::vector<LevelState> get_level_states(SquirrelVM& vm)
{
  std::vector<LevelState> results;

  sq_pushnull(vm.get_vm());
  while (SQ_SUCCEEDED(sq_next(vm.get_vm(), -2)))
  {
    //here -1 is the value and -2 is the key
    const char* result;
    if (SQ_FAILED(sq_getstring(vm.get_vm(), -2, &result)))
    {
      std::ostringstream msg;
      msg << "Couldn't get string value";
      throw SquirrelError(vm.get_vm(), msg.str());
    }
    else
    {
      LevelState level_state;
      level_state.filename = result;
      vm.get_bool("solved", level_state.solved);
      vm.get_bool("perfect", level_state.perfect);

      results.push_back(level_state);
    }

    // pops key and val before the next iteration
    sq_pop(vm.get_vm(), 2);
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

std::unique_ptr<Savegame>
Savegame::from_file(const std::string& filename)
{
  std::unique_ptr<Savegame> savegame(new Savegame(filename));
  savegame->load();
  return savegame;
}

Savegame::Progress
Savegame::progress_from_file(const std::string& filename)
{
  std::unique_ptr<Savegame> savegame(new Savegame(filename));
  savegame->load(true);
  return savegame->get_progress();
}

Savegame::Savegame(const std::string& filename) :
  m_filename(filename),
  m_progress({ -1, -1 }),
  m_player_status(new PlayerStatus(InputManager::current()->get_num_users()))
{
}

bool
Savegame::is_title_screen() const
{
  // bit of a hack, TileScreen uses a dummy savegame without a filename
  return m_filename.empty();
}

void
Savegame::load(bool progress_only)
{
  if (m_filename.empty())
  {
    log_debug << "no filename set for savegame, skipping load" << std::endl;
    return;
  }

  clear_state_table();

  if (!PHYSFS_exists(m_filename.c_str()))
  {
    log_info << m_filename << " doesn't exist, not loading state" << std::endl;
  }
  else
  {
    if (physfsutil::is_directory(m_filename))
    {
      log_info << m_filename << " is a directory, not loading state" << std::endl;
      return;
    }
    log_debug << "loading savegame from " << m_filename << std::endl;

    try
    {
      SquirrelVM& vm = SquirrelVirtualMachine::current()->get_vm();

      auto doc = ReaderDocument::from_file(m_filename);
      auto root = doc.get_root();

      if (root.get_name() != "supertux-savegame")
      {
        throw std::runtime_error("file is not a supertux-savegame file");
      }
      else
      {
        auto mapping = root.get_mapping();

        int version = 1;
        mapping.get("version", version);
        if (version != 1)
        {
          throw std::runtime_error("incompatible savegame version");
        }
        else
        {
          mapping.get("progress", m_progress.progress);
          mapping.get("total", m_progress.total);

          if (progress_only) return; // If only set to read progress, don't continue.

          boost::optional<ReaderMapping> tux;
          if (!mapping.get("tux", tux))
          {
            throw std::runtime_error("No tux section in savegame");
          }
          {
            m_player_status->read(*tux);
          }

          boost::optional<ReaderMapping> state;
          if (!mapping.get("state", state))
          {
            throw std::runtime_error("No state section in savegame");
          }
          else
          {
            sq_pushroottable(vm.get_vm());
            vm.get_table_entry("state");
            load_squirrel_table(vm.get_vm(), -1, *state);
            sq_pop(vm.get_vm(), 2);
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
  SquirrelVM& vm = SquirrelVirtualMachine::current()->get_vm();

  // delete existing state table, if it exists
  sq_pushroottable(vm.get_vm());
  {
    // create a new empty state table
    vm.create_empty_table("state");
  }
  sq_pop(vm.get_vm(), 1);
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
    if (!PHYSFS_exists(dirname.c_str()))
    {
      if (!PHYSFS_mkdir(dirname.c_str()))
      {
        std::ostringstream msg;
        msg << "Couldn't create directory for savegames '"
            << dirname << "': " <<PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
        throw std::runtime_error(msg.str());
      }
    }

    if (!physfsutil::is_directory(dirname))
    {
      std::ostringstream msg;
      msg << "Savegame path '" << dirname << "' is not a directory";
      throw std::runtime_error(msg.str());
    }
  }

  SquirrelVM& vm = SquirrelVirtualMachine::current()->get_vm();

  Writer writer(m_filename);

  writer.start_list("supertux-savegame");
  writer.write("version", 1);

  using namespace worldmap;
  if (WorldMap::current() != nullptr)
  {
    writer.write("title", WorldMap::current()->get_title());
    writer.write("progress", static_cast<int>(WorldMap::current()->solved_level_count()));
    writer.write("total", static_cast<int>(WorldMap::current()->level_count()));
  }

  writer.start_list("tux");
  m_player_status->write(writer);
  writer.end_list("tux");

  writer.start_list("state");

  sq_pushroottable(vm.get_vm());
  try
  {
    vm.get_table_entry("state"); // Push "state"
    save_squirrel_table(vm.get_vm(), -1, writer);
    sq_pop(vm.get_vm(), 1); // Pop "state"
  }
  catch(const std::exception&)
  {
  }
  sq_pop(vm.get_vm(), 1); // Pop root table
  writer.end_list("state");

  writer.end_list("supertux-savegame");
}

std::vector<std::string>
Savegame::get_worldmaps()
{
  std::vector<std::string> worlds;

  SquirrelVM& vm = SquirrelVirtualMachine::current()->get_vm();
  SQInteger oldtop = sq_gettop(vm.get_vm());

  try
  {
    sq_pushroottable(vm.get_vm());
    vm.get_table_entry("state");
    vm.get_or_create_table_entry("worlds");
    worlds = vm.get_table_keys();
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm.get_vm(), oldtop);

  // ensure that the loaded worldmap names have their canonical form
  std::transform(worlds.begin(), worlds.end(), worlds.begin(), physfsutil::realpath);

  return worlds;
}

WorldmapState
Savegame::get_worldmap_state(const std::string& name)
{
  WorldmapState result;

  SquirrelVM& vm = SquirrelVirtualMachine::current()->get_vm();
  SQInteger oldtop = sq_gettop(vm.get_vm());

  try
  {
    sq_pushroottable(vm.get_vm());
    vm.get_table_entry("state");
    vm.get_or_create_table_entry("worlds");

    // if a non-canonical entry is present, replace them with a canonical one
    if (name != "/levels/world2/worldmap.stwm") {
      std::string old_map_filename = name.substr(1);
      if (vm.has_property(old_map_filename.c_str())) {
        vm.rename_table_entry(old_map_filename.c_str(), name.c_str());
      }
    }

    vm.get_or_create_table_entry(name);
    vm.get_or_create_table_entry("levels");

    result.level_states = get_level_states(vm);
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm.get_vm(), oldtop);

  return result;
}

std::vector<std::string>
Savegame::get_levelsets()
{
  std::vector<std::string> results;

  SquirrelVM& vm = SquirrelVirtualMachine::current()->get_vm();
  SQInteger oldtop = sq_gettop(vm.get_vm());

  try
  {
    sq_pushroottable(vm.get_vm());
    vm.get_table_entry("state");
    vm.get_or_create_table_entry("levelsets");
    results = vm.get_table_keys();
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm.get_vm(), oldtop);

  return results;
}

LevelsetState
Savegame::get_levelset_state(const std::string& basedir)
{
  LevelsetState result;

  SquirrelVM& vm = SquirrelVirtualMachine::current()->get_vm();
  SQInteger oldtop = sq_gettop(vm.get_vm());

  try
  {
    sq_pushroottable(vm.get_vm());
    vm.get_table_entry("state");
    vm.get_or_create_table_entry("levelsets");
    vm.get_or_create_table_entry(basedir);
    vm.get_or_create_table_entry("levels");

    result.level_states = get_level_states(vm);
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm.get_vm(), oldtop);

  return result;
}

void
Savegame::set_levelset_state(const std::string& basedir,
                             const std::string& level_filename,
                             bool solved)
{
  LevelsetState state = get_levelset_state(basedir);

  SquirrelVM& vm = SquirrelVirtualMachine::current()->get_vm();
  SQInteger oldtop = sq_gettop(vm.get_vm());

  try
  {
    sq_pushroottable(vm.get_vm());
    vm.get_table_entry("state");
    vm.get_or_create_table_entry("levelsets");
    vm.get_or_create_table_entry(basedir);
    vm.get_or_create_table_entry("levels");
    vm.get_or_create_table_entry(level_filename);

    bool old_solved = false;
    vm.get_bool("solved", old_solved);
    vm.store_bool("solved", solved || old_solved);
  }
  catch(const std::exception& err)
  {
    log_warning << err.what() << std::endl;
  }

  sq_settop(vm.get_vm(), oldtop);
}

/* EOF */
