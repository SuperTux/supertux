//  $Id: surface.cpp 2175 2004-11-24 19:02:49Z sik0fewl $
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

#include <cassert>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <math.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>

#include "gameconfig.hpp"
#include "physfs/physfs_sdl.hpp"
#include "video/surface.hpp"
#include "video/screen.hpp"
#include "image_texture.hpp"
#include "texture_manager.hpp"
#include "msg.hpp"

Surface::Surface(const std::string& file)
{
  texture = texture_manager->get(file);
  texture->ref();

  offsetx = 0;
  offsety = 0;
  width = static_cast<int>(texture->get_image_width());
  height = static_cast<int>(texture->get_image_height());

  flipx = false;
}

Surface::Surface(const std::string& file, int x, int y, int w, int h)
{
  texture = texture_manager->get(file);
  texture->ref();

  offsetx = x;
  offsety = y;
  width = w;
  height = h;

  flipx = false;
}

Surface::Surface(const Surface& other)
{
  texture = other.texture;
  texture->ref();

  offsetx = other.offsetx;
  offsety = other.offsety;
  width = other.width;
  height = other.height;

  flipx = other.flipx;
}

const Surface&
Surface::operator= (const Surface& other)
{
  other.texture->ref();
  texture->unref();
  texture = other.texture;

  offsetx = other.offsetx;
  offsety = other.offsety;
  width = other.width;
  height = other.height;

  flipx = other.flipx;

  return *this;
}

Surface::~Surface()
{
  texture->unref();

  for (std::list<TransformedSurface*>::iterator i = transformedSurfaces.begin(); i != transformedSurfaces.end(); i++) {
    SDL_FreeSurface((*i)->surface);
    delete (*i);
  }
}

void
Surface::hflip()
{
  flipx = !flipx;
}

void
Surface::draw(float x, float y, float alpha, DrawingEffect effect) const
{
  draw_part(0, 0, x, y, width, height, alpha, effect);
}

void
Surface::draw_part(float src_x, float src_y, float dst_x, float dst_y,
                   float width, float height, float alpha,
                   DrawingEffect effect) const
{
  alpha = 0; // FIXME: this is just so alpha gets used once

  //FIXME: support parameter "alpha"
  SDL_Surface* surface = texture->getSurface();

  // get and check SDL_Surface
  if (surface == 0) {
    msg_warning("Tried to draw NULL surface, skipped draw");
    return;
  }	

  SDL_Surface* transformedSurface = surface;

  if (flipx) effect = HORIZONTAL_FLIP;

  if (effect != NO_EFFECT) {
    transformedSurface = 0;

    // check if we have this effect buffered
    for (std::list<TransformedSurface*>::const_iterator i = transformedSurfaces.begin(); i != transformedSurfaces.end(); i++) {
      if ((*i)->effect == effect) transformedSurface = (*i)->surface;
    }

    // if not: transform and buffer
    if (!transformedSurface) {
      if (effect == HORIZONTAL_FLIP) transformedSurface = zoomSurface(surface, -1, 1, 0);
      if (effect == VERTICAL_FLIP) transformedSurface = zoomSurface(surface, 1, -1, 0);
      if (transformedSurface == 0) {
        msg_warning("No known transformation applies to surface, skipped draw");
        return;
      }	
      TransformedSurface* su = new TransformedSurface();
      su->surface = transformedSurface;
      su->effect = effect;

      transformedSurfaces.push_front(su);
    }
  }

  int ox = offsetx; if (effect == HORIZONTAL_FLIP) ox = static_cast<int>(surface->w) - (ox+static_cast<int>(width));
  int oy = offsety; if (effect == VERTICAL_FLIP) oy = static_cast<int>(surface->h) - (oy+static_cast<int>(height));
  // draw surface to screen
  SDL_Surface* screen = SDL_GetVideoSurface();

  SDL_Rect srcRect;
  srcRect.x = static_cast<int>(ox+src_x);
  srcRect.y = static_cast<int>(oy+src_y);
  srcRect.w = static_cast<int>(width);
  srcRect.h = static_cast<int>(height);

  SDL_Rect dstRect;
  dstRect.x = static_cast<int>(dst_x);
  dstRect.y = static_cast<int>(dst_y);

  SDL_BlitSurface(transformedSurface, &srcRect, screen, &dstRect);
}

