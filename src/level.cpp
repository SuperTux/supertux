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

#include <map>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <stdexcept>

#include "video/screen.hpp"
#include "msg.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"
#include "lisp/writer.hpp"
#include "level.hpp"
#include "physic.hpp"
#include "sector.hpp"
#include "tile.hpp"
#include "resources.hpp"
#include "file_system.hpp"
#include "object/gameobjs.hpp"
#include "object/camera.hpp"
#include "object/tilemap.hpp"
#include "object/coin.hpp"

// test
#include "flip_level_transformer.hpp"

using namespace std;

Level::Level()
  : name("noname"), author("Mr. X")
{
}

void
Level::load(const std::string& filepath)
{
  try {
    lisp::Parser parser;
    std::auto_ptr<lisp::Lisp> root (parser.parse(filepath));

    const lisp::Lisp* level = root->get_lisp("supertux-level");
    if(!level)
      throw std::runtime_error("file is not a supertux-level file.");

    int version = 1;
    level->get("version", version);
    if(version == 1) {
      load_old_format(*level);
      return;
    }

    lisp::ListIterator iter(level);
    while(iter.next()) {
      const std::string& token = iter.item();
      if(token == "version") {
        iter.value()->get(version);
        if(version > 2) {
          msg_warning("level format newer than application");
        }
      } else if(token == "name") {
        iter.value()->get(name);
      } else if(token == "author") {
        iter.value()->get(author);
      } else if(token == "sector") {
        Sector* sector = new Sector;
        sector->parse(*(iter.lisp()));
        add_sector(sector);
      } else {
        msg_warning("Unknown token '" << token << "' in level file");
        continue;
      }
    }
    
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Problem when reading level '" << filepath << "': " << e.what();
    throw std::runtime_error(msg.str());
  }
}

void
Level::load_old_format(const lisp::Lisp& reader)
{
  reader.get("name", name);
  reader.get("author", author);

  Sector* sector = new Sector;
  sector->parse_old_format(reader);
  add_sector(sector);
}

void
Level::save(const std::string& filename)
{
  lisp::Writer* writer = new lisp::Writer(filename);

  writer->write_comment("Level made using SuperTux's built-in Level Editor");

  writer->start_list("supertux-level");

  int version = 2;
  writer->write_int("version", version);

  writer->write_string("name", name, true);
  writer->write_string("author", author);

  for(Sectors::iterator i = sectors.begin(); i != sectors.end(); ++i) {
    Sector* sector = *i;
    writer->start_list("sector");
    sector->write(*writer);
    writer->end_list("sector");
  }

  writer->end_list("supertux-level");

  delete writer;
}

Level::~Level()
{
  for(Sectors::iterator i = sectors.begin(); i != sectors.end(); ++i)
    delete *i;
}

void
Level::add_sector(Sector* sector)
{
  Sector* test = get_sector(sector->get_name());
  if(test != 0) {
    throw std::runtime_error("Trying to add 2 sectors with same name");
  }
  sectors.push_back(sector);
}

Sector*
Level::get_sector(const std::string& name)
{
  for(Sectors::iterator i = sectors.begin(); i != sectors.end(); ++i) {
    Sector* sector = *i;
    if(sector->get_name() == name)
      return sector;
  }

  return 0;
}

size_t
Level::get_sector_count()
{
  return sectors.size();
}

Sector*
Level::get_sector(size_t num)
{
  return sectors.at(num);
}

int
Level::get_total_badguys()
{
  int total_badguys = 0;
  for(Sectors::iterator i = sectors.begin(); i != sectors.end(); ++i)
    total_badguys += (*i)->get_total_badguys();
  return total_badguys;
}

int
Level::get_total_coins()
{
  // FIXME not really correct as coins can also be inside blocks...
  int total_coins = 0;
  for(Sectors::iterator i = sectors.begin(); i != sectors.end(); ++i) {
    Sector* sector = *i;
    for(Sector::GameObjects::iterator o = sector->gameobjects.begin();
        o != sector->gameobjects.end(); ++o) {
      Coin* coin = dynamic_cast<Coin*> (*o);
      if(coin)
        total_coins++;
    }
  }
  return total_coins;
}

