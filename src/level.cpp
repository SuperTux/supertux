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

#include "app/globals.h"
#include "app/setup.h"
#include "camera.h"
#include "video/screen.h"
#include "level.h"
#include "math/physic.h"
#include "scene.h"
#include "sector.h"
#include "tile.h"
#include "utils/lispreader.h"
#include "resources.h"
#include "gameobjs.h"
#include "utils/lispwriter.h"
#include "tilemap.h"

using namespace std;

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
    delete level;
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
  reader.read_string("name", name, true);
  reader.read_string("author", author);
  reader.read_int("time", time_left);

  Sector* sector = new Sector;
  sector->parse_old_format(reader);
  add_sector(sector);
}

void
Level::save(const std::string& filename)
{
 ofstream file(filename.c_str(), ios::out);
 LispWriter* writer = new LispWriter(file);

 writer->write_comment("Level made using SuperTux's built-in Level Editor");

 writer->start_list("supertux-level");

 int version = 2;
 writer->write_int("version", version);

 writer->write_string("name", name);
 writer->write_string("author", author);
 writer->write_int("time", time_left);

 for(Sectors::iterator i = sectors.begin(); i != sectors.end(); ++i)
   {
   writer->start_list("sector");
   i->second->write(*writer);
   writer->end_list("sector");
   }

 writer->end_list("supertux-level");

 delete writer;
 file.close();
}

Level::~Level()
{
  for(Sectors::iterator i = sectors.begin(); i != sectors.end(); ++i)
    delete i->second;
}

void
Level::do_vertical_flip()
{
  for(Sectors::iterator i = sectors.begin(); i != sectors.end(); ++i)
    i->second->do_vertical_flip();
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

int
Level::get_total_badguys()
{
  int total_badguys = 0;
  for(Sectors::iterator i = sectors.begin(); i != sectors.end(); ++i)
    total_badguys += i->second->get_total_badguys();
  return total_badguys;
}

int
Level::get_total_coins()
{
  int total_coins = 0;
  for(Sectors::iterator it = sectors.begin(); it != sectors.end(); ++it)
    for(int x = 0; x < it->second->solids->get_width(); x++)
      for(int y = 0; y < it->second->solids->get_height(); y++)
        if(it->second->solids->get_tile(x,y)->attributes & Tile::COIN)
          total_coins++;
  return total_coins;
}
