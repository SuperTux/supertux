//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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

#include <assert.h>
#include "video/drawing_context.h"
#include "app/setup.h"
#include "app/globals.h"
#include "utils/lispreader.h"
#include "tile.h"
#include "tile_manager.h"
#include "scene.h"

TileManager* TileManager::instance_  = 0;

TileManager::TileManager()
{
  std::string filename = datadir + "/images/tilesets/supertux.stgt";
  load_tileset(filename);
}

TileManager::~TileManager()
{
  for(Tiles::iterator i = tiles.begin(); i != tiles.end(); ++i)
    delete *i;
}

void TileManager::load_tileset(std::string filename)
{
  // free old tiles
  for(Tiles::iterator i = tiles.begin(); i != tiles.end(); ++i)
    delete *i;
  tiles.clear();
 
  lisp_object_t* root_obj = lisp_read_from_file(filename);

  if (!root_obj)
    Termination::abort("Couldn't load file", filename);

  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-tiles") != 0)
    assert(false);

  lisp_object_t* cur = lisp_cdr(root_obj);
  int tileset_id = 0;

  while(!lisp_nil_p(cur)) {
    lisp_object_t* element = lisp_car(cur);

    if (strcmp(lisp_symbol(lisp_car(element)), "tile") == 0)
      {
        LispReader reader(lisp_cdr(element));

        Tile* tile = new Tile;
        tile->parse(reader);

        while(tile->id >= tiles.size()) {
            tiles.push_back(0);
        }
        tiles[tile->id] = tile;
      }
    else if (strcmp(lisp_symbol(lisp_car(element)), "tileset") == 0)
      {
        LispReader reader(lisp_cdr(element));
        std::string filename;
        reader.read_string("file", filename);
        filename = datadir + "/images/tilesets/" + filename;
        load_tileset(filename);
      }
    else if (strcmp(lisp_symbol(lisp_car(element)), "tilegroup") == 0)
      {
        TileGroup new_;
        LispReader reader(lisp_cdr(element));
        reader.read_string("name", new_.name);
        reader.read_int_vector("tiles", new_.tiles);	      
        tilegroups.insert(new_).first;
      }
    else if (strcmp(lisp_symbol(lisp_car(element)), "properties") == 0)
      {
        LispReader reader(lisp_cdr(element));
        reader.read_int("id", tileset_id);
        tileset_id *= 1000;
      }
    else
      {
        std::cerr << "Unknown symbol: " << 
          lisp_symbol(lisp_car(element)) << "\n";
      }

    cur = lisp_cdr(cur);
  }

  lisp_free(root_obj);
}

