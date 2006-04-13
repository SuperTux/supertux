//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

void fillrect(float x, float y, float w, float h, const Color& col)
{
  if(w < 0) {
    x += w;
    w = -w;
  }
  if(h < 0) {
    y += h;
    h = -h;
  }

  glColor4f(col.red, col.green, col.blue, col.alpha);

  glDisable(GL_TEXTURE_2D);
  glBegin(GL_POLYGON);
  glVertex2f(x, y);
  glVertex2f(x+w, y);
  glVertex2f(x+w, y+h);
  glVertex2f(x, y+h);
  glEnd();
  glEnable(GL_TEXTURE_2D);

  glColor4f(0, 0, 0, 1);
}

void fadeout(float fade_time)
{
  float alpha_inc  = LOOP_DELAY / fade_time;
  Color c(0, 0, 0, alpha_inc);
  float alpha = 1.0;

  while(alpha >= 0) {
    alpha -= alpha_inc;
    fillrect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, c);
    // left side
    
    SDL_GL_SwapBuffers();
    sound_manager->update();
    
    SDL_Delay(int(LOOP_DELAY));
    alpha -= alpha_inc; 
  }

  fillrect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color());
}

void shrink_fade(const Vector& point, float fade_time)
{
  float left_inc  = point.x / (fade_time / LOOP_DELAY);
  float right_inc = (SCREEN_WIDTH - point.x) / (fade_time / LOOP_DELAY);
  float up_inc    = point.y / (fade_time / LOOP_DELAY);
  float down_inc  = (SCREEN_HEIGHT - point.y) / (fade_time / LOOP_DELAY);
                                                                                
  float left_cor = 0, right_cor = 0, up_cor = 0, down_cor = 0;
  Color c;
                                                                                
  while(left_cor < point.x && right_cor < SCREEN_WIDTH - point.x &&
      up_cor < point.y && down_cor < SCREEN_HEIGHT - point.y) {
    left_cor  += left_inc;
    right_cor += right_inc;
    up_cor    += up_inc;
    down_cor  += down_inc;
                                                                                
    fillrect(0, 0, left_cor, SCREEN_HEIGHT, c);  // left side
    fillrect(SCREEN_WIDTH - right_cor, 0, right_cor, SCREEN_HEIGHT, c);  // right side
    fillrect(0, 0, SCREEN_WIDTH, up_cor, c);  // up side
    fillrect(0, SCREEN_HEIGHT - down_cor, SCREEN_WIDTH, down_cor+1, c);  // down side

    SDL_GL_SwapBuffers();
  
    sound_manager->update();
    SDL_Delay(int(LOOP_DELAY));
  }
}

