//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2014 Ingo Ruhnke <grumbel@gmx.de>
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

#include "supertux/world_state.hpp"

#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "scripting/serialize.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/player_status.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "worldmap/worldmap.hpp"

WorldState::WorldState() :
  m_player_status(new PlayerStatus)
{
}

void
WorldState::load(const std::string& filename)
{
  if(!PHYSFS_exists(filename.c_str()))
  {
    log_info << filename << ": doesn't exist, not loading state" << std::endl;
  }
  else
  {
    try
    {
      HSQUIRRELVM vm = scripting::global_vm;

      lisp::Parser parser;
      const lisp::Lisp* root = parser.parse(filename);

      const lisp::Lisp* lisp = root->get_lisp("supertux-savegame");
      if(lisp == NULL)
      {
        throw std::runtime_error("file is not a supertux-savegame file");
      }
      else
      {
        int version = 1;
        lisp->get("version", version);
        if(version != 1)
        {
          throw std::runtime_error("incompatible savegame version");
        }
        else
        {
          const lisp::Lisp* tux = lisp->get_lisp("tux");
          if(tux == NULL)
          {
            throw std::runtime_error("No tux section in savegame");
          }
          {
            m_player_status->read(*tux);
          }

          const lisp::Lisp* state = lisp->get_lisp("state");
          if(state == NULL)
          {
            throw std::runtime_error("No state section in savegame");
          }
          else
          {
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
WorldState::save(const std::string& filename)
{
  { // make sure the savegame directory exists
    std::string dirname = FileSystem::dirname(filename);
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

  lisp::Writer writer(filename);

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

/* EOF */
