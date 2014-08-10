//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include <algorithm>

#include "lisp/parser.hpp"
#include "lisp/writer.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "scripting/serialize.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/globals.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/player_status.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/reader.hpp"
#include "util/string_util.hpp"
#include "worldmap/worldmap.hpp"

std::unique_ptr<World>
World::load(const std::string& filename)
{
  std::unique_ptr<World> world(new World);
  world->load_(filename);
  return std::move(world);
}

World::World() :
  m_worldmap_filename(),
  m_levels(),
  m_basedir(),
  m_savegame_filename(),
  m_world_thread(),
  m_title(),
  m_description(),
  m_player_status(new PlayerStatus),
  m_hide_from_contribs(false),
  m_is_levelset(true)
{
  sq_resetobject(&m_world_thread);
}

World::~World()
{
  sq_release(scripting::global_vm, &m_world_thread);
}

void
World::set_savegame_filename(const std::string& filename)
{
  m_savegame_filename = filename;

  // make sure the savegame directory exists
  std::string dirname = FileSystem::dirname(filename);
  if(!PHYSFS_exists(dirname.c_str())) {
    if(!PHYSFS_mkdir(dirname.c_str())) {
      std::ostringstream msg;
      msg << "Couldn't create directory for savegames '"
          << dirname << "': " <<PHYSFS_getLastError();
      throw std::runtime_error(msg.str());
    }
  }

  if(!PHYSFS_isDirectory(dirname.c_str())) {
    std::ostringstream msg;
    msg << "Savegame path '" << dirname << "' is not a directory";
    throw std::runtime_error(msg.str());
  }
}

void
World::load_(const std::string& filename)
{
  m_basedir = FileSystem::dirname(filename);
  m_worldmap_filename = m_basedir + "worldmap.stwm";

  lisp::Parser parser;
  const lisp::Lisp* root = parser.parse(filename);

  const lisp::Lisp* info = root->get_lisp("supertux-world");
  if(info == NULL)
    info = root->get_lisp("supertux-level-subset");
  if(info == NULL)
    throw std::runtime_error("File is not a world or levelsubset file");

  m_hide_from_contribs = false;
  m_is_levelset = true;

  info->get("title", m_title);
  info->get("description", m_description);
  info->get("levelset", m_is_levelset);
  info->get("hide-from-contribs", m_hide_from_contribs);

  // Level info file doesn't define any levels, so read the
  // directory to see what we can find

  std::string path = m_basedir;
  char** files = PHYSFS_enumerateFiles(path.c_str());
  if(!files)
  {
    log_warning << "Couldn't read subset dir '" << path << "'" << std::endl;
    return;
  }

  for(const char* const* filename = files; *filename != 0; ++filename)
  {
    if(StringUtil::has_suffix(*filename, ".stl"))
    {
      Level level;
      level.fullpath = path + *filename;
      level.name = *filename;
      m_levels.push_back(level);
    }
  }
  PHYSFS_freeList(files);

  std::sort(m_levels.begin(), m_levels.end(),
            [](const Level& lhs, const Level& rhs)
            {
              return StringUtil::numeric_less(lhs.fullpath, rhs.fullpath);
            });
}

void
World::run()
{
  // create new squirrel table for persistent game state
  HSQUIRRELVM vm = scripting::global_vm;

  sq_pushroottable(vm);
  sq_pushstring(vm, "state", -1);
  sq_newtable(vm);
  if(SQ_FAILED(sq_createslot(vm, -3)))
  {
    throw scripting::SquirrelError(vm, "Couldn't create state table");
  }
  else
  {
    sq_pop(vm, 1);

    load_state();

    std::string filename = m_basedir + "/world.nut";
    try
    {
      IFileStreambuf ins(filename);
      std::istream in(&ins);

      sq_release(scripting::global_vm, &m_world_thread);
      m_world_thread = scripting::create_thread(scripting::global_vm);
      scripting::compile_and_run(scripting::object_to_vm(m_world_thread), in, filename);
    }
    catch(const std::exception& )
    {
      // fallback: try to load worldmap worldmap.stwm
      g_screen_manager->push_screen(std::unique_ptr<Screen>(
                                      new worldmap::WorldMap(m_basedir + "worldmap.stwm",
                                                             get_player_status())));
    }
  }
}

void
World::save_state()
{
  HSQUIRRELVM vm = scripting::global_vm;

  lisp::Writer writer(m_savegame_filename);

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
  if(SQ_SUCCEEDED(sq_get(vm, -2))) {
    scripting::save_squirrel_table(vm, -1, writer);
    sq_pop(vm, 1);
  }
  sq_pop(vm, 1);
  writer.end_list("state");

  writer.end_list("supertux-savegame");
}

void
World::load_state()
{
  if(!PHYSFS_exists(m_savegame_filename.c_str()))
  {
    log_info << m_savegame_filename << ": doesn't exist, not loading state" << std::endl;
  }
  else
  {
    try
    {
      HSQUIRRELVM vm = scripting::global_vm;

      lisp::Parser parser;
      const lisp::Lisp* root = parser.parse(m_savegame_filename);

      const lisp::Lisp* lisp = root->get_lisp("supertux-savegame");
      if(lisp == NULL)
        throw std::runtime_error("file is not a supertux-savegame file");

      int version = 1;
      lisp->get("version", version);
      if(version != 1)
        throw std::runtime_error("incompatible savegame version");

      const lisp::Lisp* tux = lisp->get_lisp("tux");
      if(tux == NULL)
        throw std::runtime_error("No tux section in savegame");
      m_player_status->read(*tux);

      const lisp::Lisp* state = lisp->get_lisp("state");
      if(state == NULL)
        throw std::runtime_error("No state section in savegame");

      sq_pushroottable(vm);
      sq_pushstring(vm, "state", -1);
      if(SQ_FAILED(sq_deleteslot(vm, -2, SQFalse)))
        sq_pop(vm, 1);

      sq_pushstring(vm, "state", -1);
      sq_newtable(vm);
      scripting::load_squirrel_table(vm, -1, *state);
      if(SQ_FAILED(sq_createslot(vm, -3)))
        throw std::runtime_error("Couldn't create state table");
      sq_pop(vm, 1);
    }
    catch(const std::exception& e)
    {
      log_fatal << "Couldn't load savegame: " << e.what() << std::endl;
    }
  }
}

const std::string&
World::get_level_filename(unsigned int i) const
{
  return m_levels[i].fullpath;
}

unsigned int
World::get_num_levels() const
{
  return m_levels.size();
}

int
World::get_num_solved_levels() const
{
  int num_solved_levels = 0;

  HSQUIRRELVM vm = scripting::global_vm;
  int oldtop = sq_gettop(vm);

  sq_pushroottable(vm);
  sq_pushstring(vm, "state", -1);
  if(SQ_FAILED(sq_get(vm, -2)))
  {
    log_warning << "failed to get 'state' table" << std::endl;
  }
  else
  {
    sq_pushstring(vm, "worlds", -1);
    if(SQ_FAILED(sq_get(vm, -2)))
    {
      log_warning << "failed to get 'state.worlds' table" << std::endl;
    }
    else
    {
      sq_pushstring(vm, m_worldmap_filename.c_str(), -1);
      if(SQ_FAILED(sq_get(vm, -2)))
      {
        log_warning << "failed to get state.worlds['" << m_worldmap_filename << "']" << std::endl;
      }
      else
      {
        sq_pushstring(vm, "levels", -1);
        if(SQ_FAILED(sq_get(vm, -2)))
        {
          log_warning << "failed to get state.worlds['" << m_worldmap_filename << "'].levels" << std::endl;
        }
        else
        {
          for(auto level : m_levels)
          {
            sq_pushstring(vm, level.name.c_str(), -1);
            if(SQ_FAILED(sq_get(vm, -2)))
            {
              log_warning << "failed to get state.worlds['" << m_worldmap_filename << "'].levels['"
                          << level.name << "']" << std::endl;
            }
            else
            {
              bool solved = scripting::read_bool(vm, "solved");
              if (solved)
              {
                num_solved_levels += 1;
              }
              sq_pop(vm, 1);
            }
          }
        }
      }
    }
  }

  sq_settop(vm, oldtop);

  return num_solved_levels;
}

const std::string&
World::get_basedir() const
{
  return m_basedir;
}

const std::string&
World::get_title() const
{
  return m_title;
}

/* EOF */
