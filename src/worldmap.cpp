//  $Id$
//
//  Pingus - A free Lemmings clone
//  Copyright (C) 2002 Ingo Ruhnke <grumbel@gmx.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <vector>
#include <assert.h>
#include "texture.h"
#include "screen.h"
#include "lispreader.h"
#include "worldmap.h"

namespace WorldMapNS {

TileManager* TileManager::instance_  = 0;

TileManager::TileManager()
{
  lisp_stream_t stream;
  FILE* in = fopen(DATA_PREFIX "images/worldmap/antarctica.scm", "r");
  assert(in);
  lisp_stream_init_file (&stream, in);
  lisp_object_t* root_obj = lisp_read (&stream);
  
  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-worldmap-tiles") == 0)
    {
      lisp_object_t* cur = lisp_cdr(root_obj);

      while(!lisp_nil_p(cur))
        {
          lisp_object_t* element = lisp_car(cur);

          if (strcmp(lisp_symbol(lisp_car(element)), "tile") == 0)
            {
              int id = 0;
              std::string filename = "<invalid>";

              Tile* tile = new Tile;             
              tile->north = true;
              tile->east  = true;
              tile->south = true;
              tile->west  = true;
              tile->stop  = true;
  
              LispReader reader(lisp_cdr(element));
              reader.read_int("id",  &id);
              reader.read_bool("north", &tile->north);
              reader.read_bool("south", &tile->south);
              reader.read_bool("west",  &tile->west);
              reader.read_bool("east",  &tile->east);
              reader.read_bool("stop",  &tile->stop);
              reader.read_string("image",  &filename);

              texture_load(&tile->sprite, 
                           const_cast<char*>((std::string(DATA_PREFIX "/images/worldmap/") + filename).c_str()), 
                           USE_ALPHA);

              if (id >= tiles.size())
                tiles.resize(id+1);

              tiles[id] = tile;
            }
          else
            {
              puts("Unhandled symbol");
            }

          cur = lisp_cdr(cur);
        }
    }
  else
    {
      assert(0);
    }
}

Tile*
TileManager::get(int i)
{
  assert(i >=0 && i < tiles.size());
  return tiles[i];
}

WorldMap::WorldMap()
{
  quit = false;
  width  = 20;
  height = 15;
  tux_moving = false;

  texture_load(&tux_sprite, DATA_PREFIX "/images/worldmap/tux.png", USE_ALPHA);

  tux_offset = 0;

  tux_tile_pos.x = 0;
  tux_tile_pos.y = 0;

  input_direction = NONE;
  tux_direction = NONE;
  enter_level = false;

  load_map();
}

WorldMap::~WorldMap()
{
}

void
WorldMap::load_map()
{
  lisp_stream_t stream;
  FILE* in = fopen(DATA_PREFIX "levels/default/worldmap.scm", "r");
  assert(in);
  lisp_stream_init_file (&stream, in);
  lisp_object_t* root_obj = lisp_read (&stream);
  
  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-worldmap") == 0)
    {
      lisp_object_t* cur = lisp_cdr(root_obj);

      while(!lisp_nil_p(cur))
        {
          lisp_object_t* element = lisp_car(cur);

          if (strcmp(lisp_symbol(lisp_car(element)), "tilemap") == 0)
            {
              LispReader reader(lisp_cdr(element));
              reader.read_int("width",  &width);
              reader.read_int("height", &height);
              reader.read_int_vector("data", &tilemap);
            }
          else
            {
              
            }
          
          cur = lisp_cdr(cur);
        }
    }
}

void
WorldMap::get_input()
{
  SDL_Event event;

  enter_level = false;

  while (SDL_PollEvent(&event))
    {
      switch(event.type)
        {
        case SDL_QUIT:
          quit = true;
          break;
          
        case SDL_KEYDOWN:
          switch(event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
              quit = true;
              break;
            case SDLK_LCTRL:
            case SDLK_RETURN:
              if (!tux_moving)
                enter_level = true;
              break;
            }
          break;
        }
    }

  Uint8 *keystate = SDL_GetKeyState(NULL);
  
  input_direction = NONE;
  
  if (keystate[SDLK_LEFT])
    input_direction = WEST;
  else if (keystate[SDLK_RIGHT])
    input_direction = EAST;
  else if (keystate[SDLK_UP])
    input_direction = NORTH;
  else if (keystate[SDLK_DOWN])
    input_direction = SOUTH;
}

void
WorldMap::update()
{
  float speed = 4.5;

  if (enter_level)
    {
      puts("Enter the current level");
    }
  else
    {
      if (!tux_moving)
        {
          // FIXME: Cleanup, seperate tux
          switch(input_direction)
            {
            case WEST:
              if (at(tux_tile_pos)->west)
                {
                  tux_tile_pos.x -= 1;
                  tux_moving = true;
                  tux_direction = input_direction;
                }
              break;
            case EAST:
              if (at(tux_tile_pos)->east)
                {
                  tux_tile_pos.x += 1;
                  tux_moving = true;
                  tux_direction = input_direction;
                }
              break;
            case NORTH:
              if (at(tux_tile_pos)->north)
                {
                  tux_tile_pos.y -= 1;
                  tux_moving = true;
                  tux_direction = input_direction;
                }
              break;
            case SOUTH:
              if (at(tux_tile_pos)->south)
                {
                  tux_tile_pos.y += 1;
                  tux_moving = true;
                  tux_direction = input_direction;
                }
              break;
            case NONE:
              tux_moving = false;
              tux_offset = 0;
              tux_direction = input_direction;
              break;
            }
        }
      else
        {
          tux_offset += speed;

          if (tux_offset > 32)
            {
              tux_offset -= 32;

              if (at(tux_tile_pos)->stop)
                {
                  tux_direction = NONE;
                  tux_moving = false;
                }
              else
                {
                  // FIXME: Cleanup, seperate tux
                  switch(tux_direction)
                    {
                    case WEST:
                      if (at(tux_tile_pos)->west)
                        tux_tile_pos.x -= 1;
                      break;
                    case EAST:
                      if (at(tux_tile_pos)->east)
                        tux_tile_pos.x += 1;
                      break;
                    case NORTH:
                      if (at(tux_tile_pos)->north)
                        tux_tile_pos.y -= 1;
                      break;
                    case SOUTH:
                      if (at(tux_tile_pos)->south)
                        tux_tile_pos.y += 1;
                      break;
                    }                      
                }
            }
        }
    }
}

Tile*
WorldMap::at(Point p)
{
  assert(p.x >= 0 
         && p.x < width
         && p.y >= 0
         && p.y < height);
  return TileManager::instance()->get(tilemap[width * p.y + p.x]);
}

void
WorldMap::draw()
{
  for(int y = 0; y < height; ++y)
    for(int x = 0; x < width; ++x)
      {
        Tile* tile = at(Point(x, y));
        texture_draw(&tile->sprite, x*32, y*32, NO_UPDATE);
      }

  
  float x = tux_tile_pos.x * 32;
  float y = tux_tile_pos.y * 32;

  switch(tux_direction)
    {
    case WEST:
      x -= tux_offset - 32;
      break;
    case EAST:
      x += tux_offset - 32;
      break;
    case NORTH:
      y -= tux_offset - 32;
      break;
    case SOUTH:
      y += tux_offset - 32;
      break;
    }

  texture_draw(&tux_sprite, (int)x, (int)y, NO_UPDATE);
  flipscreen();
}

void
WorldMap::display()
{
  quit = false;

  while(!quit) {
    draw();
    get_input();
    update();
    SDL_Delay(20);
  }
}

} // namespace WorldMapNS

void worldmap_run()
{
  WorldMapNS::WorldMap worldmap;
  
  worldmap.display();
}

/* EOF */
