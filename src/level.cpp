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

#include <map>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "globals.h"
#include "setup.h"
#include "camera.h"
#include "screen/screen.h"
#include "level.h"
#include "physic.h"
#include "scene.h"
#include "sector.h"
#include "tile.h"
#include "lispreader.h"
#include "resources.h"
#include "gameobjs.h"
#include "lispwriter.h"

using namespace std;

LevelSubset::LevelSubset()
    : image(0), levels(0)
{
}

LevelSubset::~LevelSubset()
{
  delete image;
}

void LevelSubset::create(const std::string& subset_name)
{
  Level new_lev;
  LevelSubset new_subset;
  new_subset.name = subset_name;
  new_subset.title = "Unknown Title";
  new_subset.description = "No description so far.";
  new_subset.save();
  //new_lev.save(subset_name, 1, 0);
}

void LevelSubset::parse (lisp_object_t* cursor)
{
  while(!lisp_nil_p(cursor))
    {
      lisp_object_t* cur = lisp_car(cursor);
      char *s;

      if (!lisp_cons_p(cur) || !lisp_symbol_p (lisp_car(cur)))
        {
          printf("Not good");
        }
      else
        {
          if (strcmp(lisp_symbol(lisp_car(cur)), "title") == 0)
            {
              if(( s = lisp_string(lisp_car(lisp_cdr(cur)))) != NULL)
                {
                  title = s;
                }
            }
          else if (strcmp(lisp_symbol(lisp_car(cur)), "description") == 0)
            {
              if(( s = lisp_string(lisp_car(lisp_cdr(cur)))) != NULL)
                {
                  description = s;
                }
            }
        }
      cursor = lisp_cdr (cursor);
    }
}

void LevelSubset::load(const char* subset)
{
  FILE* fi;
  char filename[1024];
  char str[1024];
  int i;
  lisp_object_t* root_obj = 0;

  name = subset;

  snprintf(filename, 1024, "%s/levels/%s/info", st_dir, subset);
  if(!faccessible(filename))
    snprintf(filename, 1024, "%s/levels/%s/info", datadir.c_str(), subset);
  if(faccessible(filename))
    {
      fi = fopen(filename, "r");
      if (fi == NULL)
        {
          perror(filename);
        }
      lisp_stream_t stream;
      lisp_stream_init_file (&stream, fi);
      root_obj = lisp_read (&stream);

      if (root_obj->type == LISP_TYPE_EOF || root_obj->type == LISP_TYPE_PARSE_ERROR)
        {
          printf("World: Parse Error in file %s", filename);
        }

      lisp_object_t* cur = lisp_car(root_obj);

      if (!lisp_symbol_p (cur))
        {
          printf("World: Read error in %s",filename);
        }

      if (strcmp(lisp_symbol(cur), "supertux-level-subset") == 0)
        {
          parse(lisp_cdr(root_obj));

        }

      lisp_free(root_obj);
      fclose(fi);

      snprintf(str, 1024, "%s.png", filename);
      if(faccessible(str))
        {
          delete image;
          image = new Surface(str,IGNORE_ALPHA);
        }
      else
        {
          snprintf(filename, 1024, "%s/images/status/level-subset-info.png", datadir.c_str());
          delete image;
          image = new Surface(filename,IGNORE_ALPHA);
        }
    }

  for(i=1; i != -1; ++i)
    {
      /* Get the number of levels in this subset */
      snprintf(filename, 1024, "%s/levels/%s/level%d.stl", st_dir, subset,i);
      if(!faccessible(filename))
        {
          snprintf(filename, 1024, "%s/levels/%s/level%d.stl", datadir.c_str(), subset,i);
          if(!faccessible(filename))
            break;
        }
    }
  levels = --i;
}

void
LevelSubset::save()
{
  FILE* fi;
  string filename;

  /* Save data file: */
  filename = "/levels/" + name + "/";

  fcreatedir(filename.c_str());
  filename = string(st_dir) + "/levels/" + name + "/info";
  if(!fwriteable(filename.c_str()))
    filename = datadir + "/levels/" + name + "/info";
  if(fwriteable(filename.c_str()))
    {
      fi = fopen(filename.c_str(), "w");
      if (fi == NULL)
        {
          perror(filename.c_str());
        }

      /* Write header: */
      fprintf(fi,";SuperTux-Level-Subset\n");
      fprintf(fi,"(supertux-level-subset\n");

      /* Save title info: */
      fprintf(fi,"  (title \"%s\")\n", title.c_str());

      /* Save the description: */
      fprintf(fi,"  (description \"%s\")\n", description.c_str());

      fprintf( fi,")");
      fclose(fi);
    }
}

std::string
LevelSubset::get_level_filename(unsigned int num)
{
  char filename[1024];
                                                                                
  // Load data file:
  snprintf(filename, 1024, "%s/levels/%s/level%d.stl", st_dir,
      name.c_str(), num);
  if(!faccessible(filename))
    snprintf(filename, 1024, "%s/levels/%s/level%d.stl", datadir.c_str(),
        name.c_str(), num);

  return std::string(filename);
}

//---------------------------------------------------------------------------

Level::Level()
  : name("noname"), author("mr. x"), time_left(500)
{
}

void
Level::load(const std::string& filename)
{
  LispReader* level = LispReader::load(filename, "supertux-level");

  int version = 1;
  level->read_int("version", version);
  if(version == 1) {
    load_old_format(*level);
    return;
  }

  for(lisp_object_t* cur = level->get_lisp(); !lisp_nil_p(cur);
      cur = lisp_cdr(cur)) {
    std::string token = lisp_symbol(lisp_car(lisp_car(cur)));
    lisp_object_t* data = lisp_car(lisp_cdr(lisp_car(cur)));
    LispReader reader(lisp_cdr(lisp_car(cur)));

    if(token == "name") {
      name = lisp_string(data);
    } else if(token == "author") {
      author = lisp_string(data);
    } else if(token == "time") {
      time_left = lisp_integer(data);
    } else if(token == "sector") {
      Sector* sector = new Sector;
      sector->parse(reader);
      add_sector(sector);
    } else {
      std::cerr << "Unknown token '" << token << "' in level file.\n";
      continue;
    }
  }
  
  delete level;
}

void
Level::load_old_format(LispReader& reader)
{
  reader.read_string("name", name);
  reader.read_string("author", author);
  reader.read_int("time", time_left);

  Sector* sector = new Sector;
  sector->parse_old_format(reader);
  add_sector(sector);
}

void
Level::save(const std::string& filename)
{
#if 0
  LispReader* level = LispReader::load(filename, "supertux-level");

  int version = 1;
  level->read_int("version", version);
  if(version == 1) {
    load_old_format(*level);
    return;
  }

  for(lisp_object_t* cur = level->get_lisp(); !lisp_nil_p(cur);
      cur = lisp_cdr(cur)) {
    std::string token = lisp_symbol(lisp_car(lisp_car(cur)));
    lisp_object_t* data = lisp_car(lisp_cdr(lisp_car(cur)));
    LispReader reader(lisp_cdr(lisp_car(cur)));

    if(token == "name") {
      name = lisp_string(data);
    } else if(token == "author") {
      author = lisp_string(data);
    } else if(token == "time") {
      time_left = lisp_integer(data);
    } else if(token == "sector") {
      Sector* sector = new Sector;
      sector->parse(reader);
      add_sector(sector);
    } else {
      std::cerr << "Unknown token '" << token << "' in level file.\n";
      continue;
    }
  }
  
  delete level;
#endif
}

Level::~Level()
{
  for(Sectors::iterator i = sectors.begin(); i != sectors.end(); ++i)
    delete i->second;
}

void
Level::add_sector(Sector* sector)
{
  sectors.insert(std::make_pair(sector->get_name(), sector));       
}

Sector*
Level::get_sector(const std::string& name)
{
  Sectors::iterator i = sectors.find(name);
  if(i == sectors.end())
    return 0;

  return i->second;
}

