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

#include <assert.h>
#include <unistd.h>
#include "app/setup.h"
#include "level.h"
#include "app/globals.h"
#include "video/surface.h"
#include "level_subset.h"

using namespace SuperTux;

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
  new_subset.save();
}

void LevelSubset::read_info_file(const std::string& info_file)
{
  lisp_object_t* root_obj = lisp_read_from_file(info_file);
  lisp_object_t* cur = lisp_car(root_obj);

  if (lisp_symbol_p(cur) && strcmp(lisp_symbol(cur), "supertux-level-subset") == 0)
    {
      LispReader reader(lisp_cdr(root_obj));

      reader.read_string("title", title, true);
      reader.read_string("description", description, true);
      reader.read_string_vector("levels", levels);
    }
  else
    {
      std::cout << "LevelSubset: parse error in info file: " << info_file << std::endl;
    }

  lisp_free(root_obj);
}

void LevelSubset::load(const std::string& subset)
{
  name = subset;
  
  // Check in which directory our subset is located (ie. ~/.supertux/
  // or SUPERTUX_DATADIR)
  std::string filename;
  filename = st_dir + "/levels/" + subset + "/";
  if (access(filename.c_str(), R_OK) == 0)
    {
      directory = filename;
    }
  else
    {
      filename = datadir + "/levels/" + subset + "/";
      if (access(filename.c_str(), R_OK) == 0)
        directory = filename;
      else
        std::cout << "Error: LevelSubset: couldn't find subset: " << subset << std::endl;
    }
  
  read_info_file(directory + "info");

  if (levels.empty())
    { // Level info file doesn't define any levels, so read the
      // directory to see what we can find
      std::set<std::string> files;
  
      filename = st_dir + "/levels/" + subset + "/";
      if(access(filename.c_str(), R_OK) == 0)
        {
          files = FileSystem::read_directory(filename);
        }
      else
        {
          filename = datadir + "/levels/" + subset + "/";
          files = FileSystem::read_directory(filename);
        }
  
      for(std::set<std::string>::iterator i = files.begin(); i != files.end(); ++i)
        {
          if (has_suffix(*i, ".stl"))
            levels.push_back(*i);
        }
    }
}

void
LevelSubset::save()
{
  FILE* fi;
  std::string filename;

  /* Save data file: */
  filename = "/levels/" + name + "/";

  FileSystem::fcreatedir(filename.c_str());
  filename = std::string(st_dir) + "/levels/" + name + "/info";
  if(!FileSystem::fwriteable(filename.c_str()))
    filename = datadir + "/levels/" + name + "/info";
  if(FileSystem::fwriteable(filename.c_str()))
    {
      fi = fopen(filename.c_str(), "w");
      if (fi == NULL)
        {
          perror(filename.c_str());
        }

      /* Write header: */
      fprintf(fi,";; SuperTux-Level-Subset\n");
      fprintf(fi,"(supertux-level-subset\n");

      /* Save title info: */
      fprintf(fi,"  (title \"%s\")\n", title.c_str());

      /* Save the description: */
      fprintf(fi,"  (description \"%s\")\n", description.c_str());

      fprintf( fi,")");
      fclose(fi);
    }
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
  return directory + levels[num];
}

int
LevelSubset::get_num_levels() const
{
  return levels.size();
}

/* EOF */
