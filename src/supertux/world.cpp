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

#include "physfs/ifile_streambuf.hpp"
#include "physfs/physfs_file_system.hpp"
#include "scripting/serialize.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/player_status.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/world.hpp"
#include "supertux/savegame.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
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
    world->m_savegame_filename = stream.str();
  }

  return world;
}

std::unique_ptr<World>
World::create(const std::string& title, const std::string& desc)
{
  std::unique_ptr<World> world(new World);

  //Limit the charset to numbers and alphabet.
  std::string base = title;
  for (size_t i = 0; i < base.length(); i++) {
    if (!((base[i] >= '0' && base[i] <= '9') ||
          (base[i] >= 'A' && base[i] <= 'Z') ||
          (base[i] >= 'a' && base[i] <= 'z'))) {
      base[i] = '_';
    }
  }

  base = "levels/" + base;

  //Find a non-existing fitting directory name
  std::string dirname = base;
  if (PHYSFS_exists(dirname.c_str())) {
    int num = 1;
    do {
      num++;
      dirname = base + std::to_string(num);
    } while ( PHYSFS_exists(dirname.c_str()) );
  }

  world->create_(dirname, title, desc);

  { // generate savegame filename
    std::string worlddirname = FileSystem::basename(dirname);
    std::ostringstream stream;
    stream << "profile" << g_config->profile << "/" << worlddirname << ".stsg";
    world->m_savegame_filename = stream.str();
  }

  return world;
}

World::World() :
  m_basedir(),
  m_worldmap_filename(),
  m_savegame_filename(),
  m_title(),
  m_description(),
  m_hide_from_contribs(false),
  m_is_levelset(true)
{
}

World::~World()
{
}

void
World::load_(const std::string& directory)
{
  m_basedir = directory;
  m_worldmap_filename = m_basedir + "/worldmap.stwm";

  std::string filename = m_basedir + "/info";

  if(!PHYSFS_exists(filename.c_str()) ||
     PhysFSFileSystem::is_directory(filename))
  {
    set_default_values();
    return;
  }

  try {
    register_translation_directory(filename);
    auto doc = ReaderDocument::parse(filename);
    auto root = doc.get_root();

    if(root.get_name() != "supertux-world" &&
       root.get_name() != "supertux-level-subset")
    {
      throw std::runtime_error("File is not a world or levelsubset file");
    }

    m_hide_from_contribs = false;
    m_is_levelset = true;

    auto info = root.get_mapping();

    info.get("title", m_title);
    info.get("description", m_description);
    info.get("levelset", m_is_levelset);
    info.get("hide-from-contribs", m_hide_from_contribs);
  } catch (std::exception& e) {
    log_warning << "Failed to load " << filename << ":" << e.what() << std::endl;
    set_default_values();
  }
}

void
World::create_(const std::string& directory, const std::string& title, const std::string& desc)
{
  m_basedir = directory;
  m_worldmap_filename = m_basedir + "/worldmap.stwm";

  m_title = title;
  m_description = desc;
  m_is_levelset = true;
  m_hide_from_contribs = false;
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

void
World::save(bool retry)
{
  std::string filepath = m_basedir + "/info";

  try {

    { // make sure the levelset directory exists
      std::string dirname = FileSystem::dirname(filepath);
      if(!PHYSFS_exists(dirname.c_str()))
      {
        if(!PHYSFS_mkdir(dirname.c_str()))
        {
          std::ostringstream msg;
          msg << "Couldn't create directory for levelset '"
              << dirname << "': " <<PHYSFS_getLastError();
          throw std::runtime_error(msg.str());
        }
      }

      if(!PhysFSFileSystem::is_directory(dirname))
      {
        std::ostringstream msg;
        msg << "Levelset path '" << dirname << "' is not a directory";
        throw std::runtime_error(msg.str());
      }
    }

    Writer writer(filepath);
    writer.start_list("supertux-level-subset");

    writer.write("title", m_title, true);
    writer.write("description", m_description, true);
    writer.write("levelset", m_is_levelset);
    writer.write("hide-from-contribs", m_hide_from_contribs);

    writer.end_list("supertux-level-subset");
    log_warning << "Levelset info saved as " << filepath << "." << std::endl;
  } catch(std::exception& e) {
    if (retry) {
      std::stringstream msg;
      msg << "Problem when saving levelset info '" << filepath << "': " << e.what();
      throw std::runtime_error(msg.str());
    } else {
      log_warning << "Failed to save the levelset info, retrying..." << std::endl;
      { // create the levelset directory again
        std::string dirname = FileSystem::dirname(filepath);
        if(!PHYSFS_mkdir(dirname.c_str()))
        {
          std::ostringstream msg;
          msg << "Couldn't create directory for levelset '"
              << dirname << "': " <<PHYSFS_getLastError();
          throw std::runtime_error(msg.str());
        }
      }
      save(true);
    }
  }
}

void
World::set_default_values()
{
  m_title = "";
  m_description = "";
  m_is_levelset = true;
  m_hide_from_contribs = true;
}

/* EOF */
