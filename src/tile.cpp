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

#include <cassert>
#include <iostream>

#include "globals.h"
#include "tile.h"
#include "scene.h"
#include "lispreader.h"
#include "vector.h"
#include "screen/drawing_context.h"

Tile::Tile()
  : id(-1), attributes(0), data(0), next_tile(0), anim_speed(25)
{
}

Tile::~Tile()
{
  for(std::vector<Surface*>::iterator i = images.begin(); i != images.end();
      ++i) {
    delete *i;
  }
  for(std::vector<Surface*>::iterator i = editor_images.begin();
      i != editor_images.end(); ++i) {
    delete *i;                                                                
  }
}

int
Tile::read(LispReader& reader)
{
  if(!reader.read_int("id", id)) {
    std::cerr << "Missing tile-id.\n";
    return -1;
  }
  
  bool value;
  if(reader.read_bool("solid", value) && value)
    attributes |= SOLID;
  if(reader.read_bool("unisolid", value) && value)
    attributes |= GOAL;                            
  if(reader.read_bool("brick", value) && value)
    attributes |= BRICK;
  if(reader.read_bool("ice", value) && value)
    attributes |= ICE;
  if(reader.read_bool("water", value) && value)
    attributes |= WATER;
  if(reader.read_bool("spike", value) && value)
    attributes |= SPIKE;
  if(reader.read_bool("fullbox", value) && value)
    attributes |= FULLBOX;
  if(reader.read_bool("distro", value) && value)
    attributes |= COIN;
  if(reader.read_bool("coin", value) && value)
    attributes |= COIN;
  if(reader.read_bool("goal", value) && value)
    attributes |= GOAL;

  reader.read_int("data", data);
  reader.read_int("anim-speed", anim_speed);
  reader.read_int("next-tile", next_tile);

  std::vector<std::string> filenames;
  reader.read_string_vector("images", filenames);
  std::vector<std::string> editor_filenames;
  reader.read_string_vector("editor-images", editor_filenames);

  std::vector<int> grid;
  reader.read_int_vector("grid", grid);

  // read images
  for(std::vector<std::string>::iterator i = filenames.begin();
      i != filenames.end(); ++i) 
    {
      if (grid.size() == 4)
        {
          Surface* surface = new Surface(datadir + "/images/tilesets/" + *i,
                                         grid[0], grid[1], grid[2], grid[3],
                                         USE_ALPHA);
          images.push_back(surface);
        }
      else
        {
          Surface* surface = new Surface(datadir + "/images/tilesets/" + *i, USE_ALPHA);
          images.push_back(surface);
        }
    }

  for(std::vector<std::string>::iterator i = editor_filenames.begin();
      i != editor_filenames.end(); ++i) {
    Surface* surface 
      = new Surface(datadir + "/images/tilesets/" + *i, USE_ALPHA);
    editor_images.push_back(surface);
  }

  return id;
}

/* EOF */

