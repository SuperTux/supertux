//  $Id: level_subset.cpp 3118 2006-03-25 17:29:08Z sommer $
// 
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>

#include <stddef.h>
#include <physfs.h>
#include <stdexcept>

#include "world.hpp"
#include "file_system.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "physfs/physfs_stream.hpp"
#include "script_manager.hpp"
#include "scripting/wrapper_util.hpp"
#include "scripting/serialize.hpp"
#include "msg.hpp"

static bool has_suffix(const std::string& data, const std::string& suffix)
{
  if (data.length() >= suffix.length())
    return data.compare(data.length() - suffix.length(), suffix.length(), suffix) == 0;
  else
    return false;
}

World::World()
{
  is_levelset = true;
  hide_from_contribs = false;
}

World::~World()
{
}

void
World::set_savegame_filename(const std::string& filename)
{
  this->savegame_filename = filename;
}

void
World::load(const std::string& filename)
{
  basedir = FileSystem::dirname(filename);
  
  lisp::Parser parser;
  std::auto_ptr<lisp::Lisp> root (parser.parse(filename));

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
  info->get_vector("levels", levels);
  info->get("hide-from-contribs", hide_from_contribs);

  // Level info file doesn't define any levels, so read the
  // directory to see what we can find
      
  std::string path = basedir + "/";
  char** files = PHYSFS_enumerateFiles(path.c_str());
  if(!files) {
    msg_warning << "Couldn't read subset dir '" << path << "'" << std::endl;
    return;
  }

  for(const char* const* filename = files; *filename != 0; ++filename) {
    if(has_suffix(*filename, ".stl")) {
      levels.push_back(path + *filename);
    }
  }
  PHYSFS_freeList(files);
}

void
World::run()
{
  // create new squirrel table for persisten game state
  HSQUIRRELVM vm = ScriptManager::instance->get_vm();

  sq_pushroottable(vm);
  sq_pushstring(vm, "state", -1);
  sq_newtable(vm);
  if(SQ_FAILED(sq_createslot(vm, -3)))
    throw Scripting::SquirrelError(vm, "Couldn't create state table");
  sq_pop(vm, 1);
  
  std::string filename = basedir + "/world.nut";
  IFileStream in(filename);

  HSQUIRRELVM new_vm = ScriptManager::instance->create_thread();
  Scripting::compile_and_run(new_vm, in, filename);
}

void
World::save()
{
  lisp::Writer writer(savegame_filename);

  writer.start_list("supertux-savegame");
  writer.write_int("version", 1);

  writer.start_list("tux");
  player_status->write(writer);
  writer.end_list("tux");

  writer.start_list("state");
  HSQUIRRELVM vm = ScriptManager::instance->get_vm();
  sq_pushroottable(vm);
  sq_pushstring(vm, "state", -1);
  if(SQ_SUCCEEDED(sq_get(vm, -2))) {
    Scripting::save_squirrel_table(vm, -1, writer);
    sq_pop(vm, 1);
  }
  sq_pop(vm, 1);
  writer.end_list("state");
  
  writer.end_list("supertux-savegame");
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

