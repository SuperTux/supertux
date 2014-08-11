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
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/player_status.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/world.hpp"
#include "supertux/world_state.hpp"
#include "util/file_system.hpp"
#include "util/reader.hpp"
#include "util/string_util.hpp"
#include "worldmap/worldmap.hpp"

std::unique_ptr<World>
World::load(const std::string& directory)
{
  std::unique_ptr<World> world(new World);

  world->load_(directory);

  { // generate savegame filename
    std::string worlddirname = FileSystem::basename(directory);
    std::ostringstream stream;
    stream << "profile" << g_config->profile << "/" << worlddirname << ".stsg";
    std::string slotfile = stream.str();
    world->m_savegame_filename = stream.str();
  }

  return std::move(world);
}

World::World() :
  m_levels(),
  m_basedir(),
  m_worldmap_filename(),
  m_savegame_filename(),
  m_world_thread(),
  m_title(),
  m_description(),
  m_world_state(new WorldState),
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
World::load_(const std::string& directory)
{
  m_basedir = directory;
  m_worldmap_filename = m_basedir + "/worldmap.stwm";

  lisp::Parser parser;
  const lisp::Lisp* root = parser.parse(m_basedir + "/info");

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

  char** files = PHYSFS_enumerateFiles(m_basedir.c_str());
  if(!files)
  {
    log_warning << "Couldn't read subset dir '" << m_basedir << "'" << std::endl;
    return;
  }

  for(const char* const* filename = files; *filename != 0; ++filename)
  {
    if(StringUtil::has_suffix(*filename, ".stl"))
    {
      m_levels.push_back(*filename);
    }
  }
  PHYSFS_freeList(files);

  std::sort(m_levels.begin(), m_levels.end(), StringUtil::numeric_less);
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
                                      new worldmap::WorldMap(m_worldmap_filename,
                                                             get_player_status())));
    }
  }
}

void
World::save_state()
{
  m_world_state->save(m_savegame_filename);
}

void
World::load_state()
{
  m_world_state->load(m_savegame_filename);
}

std::string
World::get_level_filename(unsigned int i) const
{
  return FileSystem::join(m_basedir, m_levels[i]);
}

int
World::get_num_levels() const
{
  return static_cast<int>(m_levels.size());
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
            sq_pushstring(vm, level.c_str(), -1);
            if(SQ_FAILED(sq_get(vm, -2)))
            {
              log_warning << "failed to get state.worlds['" << m_worldmap_filename << "'].levels['"
                          << level << "']" << std::endl;
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

std::string
World::get_basedir() const
{
  return m_basedir;
}

std::string
World::get_title() const
{
  return m_title;
}

/* EOF */
