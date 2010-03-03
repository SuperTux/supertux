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
#include "physfs/ifile_stream.hpp"
#include "scripting/serialize.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/globals.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/player_status.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/reader.hpp"
#include "util/string_util.hpp"
#include "worldmap/worldmap.hpp"

World* World::current_ = NULL;

World::World() :
  levels(),
  basedir(),
  savegame_filename(),
  state_table(),
  world_thread(),
  title(),
  description(),
  player_status(),
  hide_from_contribs(),
  is_levelset()
{
  player_status.reset(new PlayerStatus());

  is_levelset = true;
  hide_from_contribs = false;
  sq_resetobject(&world_thread);
}

World::~World()
{
  sq_release(scripting::global_vm, &world_thread);
  if(current_ == this)
    current_ = NULL;
}

void
World::set_savegame_filename(const std::string& filename)
{
  this->savegame_filename = filename;
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
World::load(const std::string& filename)
{
  basedir = FileSystem::dirname(filename);

  lisp::Parser parser;
  const lisp::Lisp* root = parser.parse(filename);

  const lisp::Lisp* info = root->get_lisp("supertux-world");
  if(info == NULL)
    info = root->get_lisp("supertux-level-subset");
  if(info == NULL)
    throw std::runtime_error("File is not a world or levelsubset file");

  hide_from_contribs = false;
  is_levelset = true;

  info->get("title", title);
  info->get("description", description);
  info->get("levelset", is_levelset);
  info->get("hide-from-contribs", hide_from_contribs);

  // Level info file doesn't define any levels, so read the
  // directory to see what we can find

  std::string path = basedir;
  char** files = PHYSFS_enumerateFiles(path.c_str());
  if(!files) {
    log_warning << "Couldn't read subset dir '" << path << "'" << std::endl;
    return;
  }

  for(const char* const* filename = files; *filename != 0; ++filename) {
    if(StringUtil::has_suffix(*filename, ".stl")) {
      levels.push_back(path + *filename);
    }
  }
  PHYSFS_freeList(files);

  std::sort(levels.begin(), levels.end(), StringUtil::numeric_less);
}

void
World::run()
{
  using namespace scripting;

  current_ = this;

  // create new squirrel table for persistent game state
  HSQUIRRELVM vm = scripting::global_vm;

  sq_pushroottable(vm);
  sq_pushstring(vm, "state", -1);
  sq_newtable(vm);
  if(SQ_FAILED(sq_createslot(vm, -3)))
    throw scripting::SquirrelError(vm, "Couldn't create state table");
  sq_pop(vm, 1);

  load_state();

  std::string filename = basedir + "/world.nut";
  try {
    IFileStream in(filename);

    sq_release(global_vm, &world_thread);
    world_thread = create_thread(global_vm);
    compile_and_run(object_to_vm(world_thread), in, filename);
  } catch(std::exception& ) {
    // fallback: try to load worldmap worldmap.stwm
    using namespace worldmap;
    g_screen_manager->push_screen(new WorldMap(basedir + "worldmap.stwm", get_player_status()));
  }
}

void
World::save_state()
{
  using namespace scripting;

  lisp::Writer writer(savegame_filename);

  writer.start_list("supertux-savegame");
  writer.write("version", 1);

  using namespace worldmap;
  if(WorldMap::current() != NULL) {
    std::ostringstream title;
    title << WorldMap::current()->get_title();
    title << " (" << WorldMap::current()->solved_level_count()
          << "/" << WorldMap::current()->level_count() << ")";
    writer.write("title", title.str());
  }

  writer.start_list("tux");
  player_status->write(writer);
  writer.end_list("tux");

  writer.start_list("state");

  sq_pushroottable(global_vm);
  sq_pushstring(global_vm, "state", -1);
  if(SQ_SUCCEEDED(sq_get(global_vm, -2))) {
    scripting::save_squirrel_table(global_vm, -1, writer);
    sq_pop(global_vm, 1);
  }
  sq_pop(global_vm, 1);
  writer.end_list("state");

  writer.end_list("supertux-savegame");
}

void
World::load_state()
{
  using namespace scripting;

  try {
    lisp::Parser parser;
    const lisp::Lisp* root = parser.parse(savegame_filename);

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
    player_status->read(*tux);

    const lisp::Lisp* state = lisp->get_lisp("state");
    if(state == NULL)
      throw std::runtime_error("No state section in savegame");

    sq_pushroottable(global_vm);
    sq_pushstring(global_vm, "state", -1);
    if(SQ_FAILED(sq_deleteslot(global_vm, -2, SQFalse)))
      sq_pop(global_vm, 1);

    sq_pushstring(global_vm, "state", -1);
    sq_newtable(global_vm);
    load_squirrel_table(global_vm, -1, *state);
    if(SQ_FAILED(sq_createslot(global_vm, -3)))
      throw std::runtime_error("Couldn't create state table");
    sq_pop(global_vm, 1);
  } catch(std::exception& e) {
    log_debug << "Couldn't load savegame: " << e.what() << std::endl;
  }
}

const std::string&
World::get_level_filename(unsigned int i) const
{
  return levels[i];
}

unsigned int
World::get_num_levels() const
{
  return levels.size();
}

const std::string&
World::get_basedir() const
{
  return basedir;
}

const std::string&
World::get_title() const
{
  return title;
}

/* EOF */
