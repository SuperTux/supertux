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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "world.h"
#include "tile.h"
#include "gameloop.h"
#include "gameobjs.h"

void
BouncyDistro::init(float x, float y)
{
  base.x = x;
  base.y = y;
  base.ym = -2;
}

void
BouncyDistro::action(double frame_ratio)
{
  base.y = base.y + base.ym * frame_ratio;

  base.ym += 0.1 * frame_ratio;

  if (base.ym >= 0)
    World::current()->bouncy_distros.erase(static_cast<std::vector<BouncyDistro>::iterator>(this));
}

void
BouncyDistro::draw()
{
  texture_draw(&img_distro[0],
               base.x - scroll_x,
               base.y);
}


void
BrokenBrick::init(Tile* tile_, float x, float y, float xm, float ym)
{
  tile    = tile_;
  base.x  = x;
  base.y  = y;
  base.xm = xm;
  base.ym = ym;

  timer.init(true);
  timer.start(200);
}

void
BrokenBrick::action(double frame_ratio)
{
  base.x = base.x + base.xm * frame_ratio;
  base.y = base.y + base.ym * frame_ratio;

  if (!timer.check())
    World::current()->broken_bricks.erase(static_cast<std::vector<BrokenBrick>::iterator>(this));
}

void
BrokenBrick::draw()
{
  SDL_Rect src, dest;
  src.x = rand() % 16;
  src.y = rand() % 16;
  src.w = 16;
  src.h = 16;

  dest.x = (int)(base.x - scroll_x);
  dest.y = (int)base.y;
  dest.w = 16;
  dest.h = 16;
  
  if (tile->images.size() > 0)
    texture_draw_part(&tile->images[0],
                      src.x,src.y,dest.x,dest.y,dest.w,dest.h);
}

void
BouncyBrick::init(float x, float y)
{
  base.x   = x;
  base.y   = y;
  offset   = 0;
  offset_m = -BOUNCY_BRICK_SPEED;
  shape    = World::current()->get_level()->gettileid(x, y);
}

void
BouncyBrick::action(double frame_ratio)
{
  offset = (offset + offset_m * frame_ratio);

  /* Go back down? */
  if (offset < -BOUNCY_BRICK_MAX_OFFSET)
    offset_m = BOUNCY_BRICK_SPEED;


  /* Stop bouncing? */
  if (offset >= 0)
    World::current()->bouncy_bricks.erase(static_cast<std::vector<BouncyBrick>::iterator>(this));
}

void
BouncyBrick::draw()
{
  int s;
  SDL_Rect dest;
  
  if (base.x >= scroll_x - 32 &&
      base.x <= scroll_x + screen->w)
    {
      dest.x = (int)(base.x - scroll_x);
      dest.y = (int)base.y;
      dest.w = 32;
      dest.h = 32;

      Level* plevel = World::current()->get_level();

      // FIXME: overdrawing hack to clean the tile from the screen to
      // paint it later at on offseted position
      if(plevel->bkgd_image[0] == '\0')
        {
          fillrect(base.x - scroll_x, base.y,
                   32,32, 
                   plevel->bkgd_top_red, plevel->bkgd_top_green, plevel->bkgd_top_blue, 0);
// FIXME: doesn't respect the gradient, futhermore is this necessary at all??
        }
      else
        {
          s = (int)scroll_x / 30;
          texture_draw_part(&plevel->img_bkgd, dest.x + s, dest.y, 
                            dest.x, dest.y,dest.w,dest.h);
        }

      Tile::draw(base.x - scroll_x,
                 base.y + offset,
                 shape);
    }
}

void
FloatingScore::init(float x, float y, int s)
{
  base.x = x;
  base.y = y - 16;
  timer.init(true);
  timer.start(1000);
  value = s;
}

void
FloatingScore::action(double frame_ratio)
{
  base.y = base.y - 2 * frame_ratio;

  if(!timer.check())
    World::current()->floating_scores.erase(static_cast<std::vector<FloatingScore>::iterator>(this));
}

void
FloatingScore::draw()
{
  char str[10];
  sprintf(str, "%d", value);
  text_draw(&gold_text, str, (int)base.x + 16 - strlen(str) * 8, (int)base.y, 1);
}

/* EOF */

