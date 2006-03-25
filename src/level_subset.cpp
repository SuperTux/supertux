//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 SuperTux Development Team, see AUTHORS for details
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

#include <sstream>
#include <stdexcept>
#include <assert.h>
#include <unistd.h>
#include <physfs.h>
#include "level.hpp"
#include "msg.hpp"
#include "resources.hpp"
#include "file_system.hpp"
#include "video/surface.hpp"
#include "level_subset.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"

static bool has_suffix(const std::string& data, const std::string& suffix)
{
  if (data.length() >= suffix.length())
    return data.compare(data.length() - suffix.length(), suffix.length(), suffix) == 0;
  else
    return false;
}

LevelSubset::LevelSubset()
  : levels(0)
{
}

LevelSubset::~LevelSubset()
{
}

void LevelSubset::create(const std::string& subset_name)
{
  Level new_lev;
  LevelSubset new_subset;
  new_subset.name = subset_name;
  new_subset.title = "Unknown Title";
  new_subset.description = "No description so far.";
  new_subset.hide_from_contribs = false;
  new_subset.save();
}

void LevelSubset::read_info_file(const std::string& info_file)
{
  lisp::Parser parser;
  std::auto_ptr<lisp::Lisp> root (parser.parse(info_file));

  const lisp::Lisp* info = root->get_lisp("supertux-level-subset");
  if(!info)
    throw std::runtime_error("File is not a levelsubset file");

  hide_from_contribs = false;

  info->get("title", title);
  info->get("description", description);
  info->get_vector("levels", levels);
  info->get("hide-from-contribs", hide_from_contribs);
}

void LevelSubset::load(const std::string& subset)
{
  name = subset;
  
  std::string infofile = subset + "/info";
  try {
    read_info_file(infofile);
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Couldn't parse info file '" << infofile << "': " << e.what();
    throw std::runtime_error(msg.str());
  }

  // test is a worldmap exists
  has_worldmap = false;
  std::string worldmap = subset + "/worldmap.stwm";
  if(PHYSFS_exists(worldmap.c_str())) {
    has_worldmap = true;
  }

  if (levels.empty()) { 
    // Level info file doesn't define any levels, so read the
    // directory to see what we can find
      
    std::string path = subset + "/";
    char** files = PHYSFS_enumerateFiles(path.c_str());
    if(!files) {
      msg_warning("Couldn't read subset dir '" 
                << path << "'");
      return;
    }

    for(const char* const* filename = files; *filename != 0; ++filename) {
      if(has_suffix(*filename, ".stl")) {
        levels.push_back(path + *filename);
      }
    }
    PHYSFS_freeList(files);
  }
}

void
LevelSubset::save()
{
  /* Save data file: */
  std::string filename = name + "/info";
  lisp::Writer writer(filename);

  writer.start_list("supertux-level-subset");
  writer.write_string("title", title);
  writer.write_string("description", description);
  writer.write_bool("hide-from-contribs", hide_from_contribs);
  writer.end_list("supertux-level-subset");
}

void
LevelSubset::add_level(const std::string& name)
{
  levels.push_back(name);
}

std::string
LevelSubset::get_level_filename(unsigned int num)
{
  assert(num < levels.size());
  return levels[num];
}

std::string
LevelSubset::get_worldmap_filename()
{
  return std::string(name + "/worldmap.stwm");
}

int
LevelSubset::get_num_levels() const
{
  return levels.size();
}
