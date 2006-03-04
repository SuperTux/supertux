//  $Id: screen.cpp 2334 2005-04-04 16:26:14Z grumbel $
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
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
#include <config.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <assert.h>

#include <unistd.h>

#include <SDL.h>

#include "gameconfig.hpp"
#include "screen.hpp"
#include "main.hpp"
#include "video/drawing_context.hpp"
#include "audio/sound_manager.hpp"
#include "math/vector.hpp"

static const float LOOP_DELAY = 20.0;


void fillrect(float x, float y, float w, float h, int r, int g, int b, int a)
  {
    SDL_Surface* screen = SDL_GetVideoSurface();

    if(w < 0) {
      x += w;
      w = -w;
    }
    if(h < 0) {
      y += h;
      h = -h;
    }

    SDL_Rect src, rect;
    SDL_Surface *temp = NULL;

    rect.x = (int)x;
    rect.y = (int)y;
    rect.w = (int)w;
    rect.h = (int)h;

    if(a != 255) {
      temp = SDL_CreateRGBSurface(screen->flags, rect.w, rect.h, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);

      src.x = 0;
      src.y = 0;
      src.w = rect.w;
      src.h = rect.h;

      SDL_FillRect(temp, &src, SDL_MapRGB(screen->format, r, g, b));
      SDL_SetAlpha(temp, SDL_SRCALPHA, a);
      SDL_BlitSurface(temp,0,screen,&rect);
      SDL_FreeSurface(temp);
    }
    else
    SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, r, g, b));
}

void fadeout(int fade_time)
{
  float alpha_inc  = 256 / (fade_time / LOOP_DELAY);
  float alpha = 256;

  while(alpha > 0) {
    alpha -= alpha_inc;
    fillrect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0,0,0, (int)alpha_inc);  // left side
    
    SDL_Flip(SDL_GetVideoSurface());
    sound_manager->update();
    
    SDL_Delay(int(LOOP_DELAY));
  }
  fillrect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 255);
}

void shrink_fade(const Vector& point, int fade_time)
{
  float left_inc  = point.x / ((float)fade_time / LOOP_DELAY);
  float right_inc = (SCREEN_WIDTH - point.x) / ((float)fade_time / LOOP_DELAY);
  float up_inc    = point.y / ((float)fade_time / LOOP_DELAY);
  float down_inc  = (SCREEN_HEIGHT - point.y) / ((float)fade_time / LOOP_DELAY);
                                                                                
  float left_cor = 0, right_cor = 0, up_cor = 0, down_cor = 0;
                                                                                
  while(left_cor < point.x && right_cor < SCREEN_WIDTH - point.x &&
      up_cor < point.y && down_cor < SCREEN_HEIGHT - point.y) {
    left_cor  += left_inc;
    right_cor += right_inc;
    up_cor    += up_inc;
    down_cor  += down_inc;
                                                                                
    fillrect(0, 0, left_cor, SCREEN_HEIGHT, 0,0,0, 255);  // left side
    fillrect(SCREEN_WIDTH - right_cor, 0, right_cor, SCREEN_HEIGHT, 0,0,0, 255);  // right side
    fillrect(0, 0, SCREEN_WIDTH, up_cor, 0,0,0, 255);  // up side
    fillrect(0, SCREEN_HEIGHT - down_cor, SCREEN_WIDTH, down_cor+1, 0,0,0, 255);  // down side

    SDL_Flip(SDL_GetVideoSurface());
  
    sound_manager->update();
    SDL_Delay(int(LOOP_DELAY));
  }
}
