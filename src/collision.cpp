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

#include "defines.h"
#include "collision.h"
#include "bitmask.h"
#include "scene.h"
#include "world.h"
#include "level.h"
#include "tile.h"

bool rectcollision(const base_type& one, const base_type& two)
{
  return (one.x >= two.x - one.width + 1  &&
          one.x <= two.x + two.width - 1  &&
          one.y >= two.y - one.height + 1 &&
          one.y <= two.y + two.height - 1);
}

bool rectcollision_offset(const base_type& one, const base_type& two, float off_x, float off_y)
{
  return (one.x >= two.x - one.width  + off_x + 1 &&
          one.x <= two.x + two.width  + off_x - 1 &&
          one.y >= two.y - one.height + off_y + 1 &&
          one.y <= two.y + two.height + off_y - 1);
}

bool collision_object_map(const base_type& pbase)
{
  int v = (int)pbase.height / 16;
  int h = (int)pbase.width / 16;

  if(issolid(pbase.x + 1, pbase.y + 1) ||
     issolid(pbase.x + pbase.width -1, pbase.y + 1) ||
     issolid(pbase.x +1, pbase.y + pbase.height -1) ||
     issolid(pbase.x + pbase.width -1, pbase.y + pbase.height - 1))
    return true;

  for(int i = 1; i < h; ++i)
    {
      if(issolid(pbase.x + i*16,pbase.y + 1))
        return true;
    }

  for(int i = 1; i < h; ++i)
    {
      if(  issolid(pbase.x + i*16,pbase.y + pbase.height - 1))
        return true;
    }

  for(int i = 1; i < v; ++i)
    {
      if(  issolid(pbase.x + 1, pbase.y + i*16))
        return true;
    }
  for(int i = 1; i < v; ++i)
    {
      if(  issolid(pbase.x + pbase.width - 1, pbase.y + i*16))
        return true;
    }

  return false;
}

void* collision_func(const base_type& base, tiletestfunction function)
{
  for(float x = base.x; x < base.x + base.width; x += 32) {
    for(float y = base.y; y < base.y + base.height; y += 32) {
      Tile* tile = gettile(x, y);
      void* result = function(tile);
      if(result != 0)
        return result;
    }
  }

  return 0;
}

static void* test_goal_tile_function(Tile* tile)
{
  if(tile->goal)
    return tile;
  return 0;
}

Tile* collision_goal(const base_type& base)
{
  return (Tile*) collision_func(base, test_goal_tile_function);
}

void collision_swept_object_map(base_type* old, base_type* current)
{
  int steps; /* Used to speed up the collision tests, by stepping every 16pixels in the path. */
  int h;
  float lpath; /* Holds the longest path, which is either in X or Y direction. */
  float xd,yd; /* Hold the smallest steps in X and Y directions. */
  float temp, xt, yt; /* Temporary variable. */

  lpath = 0;
  xd = 0;
  yd = 0;

  if(old->x == current->x && old->y == current->y)
    {
      return;
    }
  else if(old->x == current->x && old->y != current->y)
    {
      lpath = current->y - old->y;
      if(lpath < 0)
        {
          yd = -1;
          lpath = -lpath;
        }
      else
        {
          yd = 1;
        }

      h = 1;
      xd = 0;
    }
  else if(old->x != current->x && old->y == current->y)
    {
      lpath = current->x - old->x;
      if(lpath < 0)
        {
          xd = -1;
          lpath = -lpath;
        }
      else
        {
          xd = 1;
        }
      h = 2;
      yd = 0;
    }
  else
    {
      lpath = current->x - old->x;
      if(lpath < 0)
        lpath = -lpath;
      if(current->y - old->y > lpath || old->y - current->y > lpath)
        lpath = current->y - old->y;
      if(lpath < 0)
        lpath = -lpath;
      h = 3;
      xd = (current->x - old->x) / lpath;
      yd = (current->y - old->y) / lpath;
    }

  steps = (int)(lpath / (float)16);

  old->x += xd;
  old->y += yd;

  for(float i = 0; i <= lpath; old->x += xd, old->y += yd, ++i)
    {
      if(steps > 0)
        {
          old->y += yd*16.;
          old->x += xd*16.;
          steps--;
        }

      if(collision_object_map(*old))
        {
          switch(h)
            {
            case 1:
              current->y = old->y - yd;
              while(collision_object_map(*current))
                current->y -= yd;
              break;
            case 2:
              current->x = old->x - xd;
              while(collision_object_map(*current))
                current->x -= xd;
              break;
            case 3:
              xt = current->x;
              yt = current->y;
              current->x = old->x - xd;
              current->y = old->y - yd;
              while(collision_object_map(*current))
                {
                  current->x -= xd;
                  current->y -= yd;
                }

              temp = current->x;
              current->x = xt;
              if(!collision_object_map(*current))
                break;
              current->x = temp;
              temp = current->y;
              current->y = yt;

              if(!collision_object_map(*current))
                {
                  break;
                }
              else
                {
                  current->y = temp;
                  while(!collision_object_map(*current))
                    current->y += yd;
		  current->y -= yd;
                  break;
                }

              break;
            default:
              break;
            }
          break;
        }
    }

  *old = *current;
}


Tile* gettile(float x, float y)
{
  return TileManager::instance()->get(World::current()->get_level()->gettileid(x, y));
}

bool issolid(float x, float y)
{
  Tile* tile = gettile(x,y);
  return tile && tile->solid;
}

bool isbrick(float x, float y)
{
  Tile* tile = gettile(x,y);
  return tile && tile->brick;
}

bool isice(float x, float y)
{
  Tile* tile = gettile(x,y);
  return tile && tile->ice;
}

bool isfullbox(float x, float y)
{
  Tile* tile = gettile(x,y);
  return tile && tile->fullbox;
}

bool isdistro(float x, float y)
{
  Tile* tile = gettile(x,y);
  return tile && tile->distro;
}

/* EOF */


