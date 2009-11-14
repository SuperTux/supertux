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
#ifndef SUPERTUX_SDL_LIGHTMAP_H
#define SUPERTUX_SDL_LIGHTMAP_H

#include <SDL_video.h>

#include "lightmap.hpp"

class Color;
struct DrawingRequest;

namespace SDL
{
  class Lightmap : public ::Lightmap
  {
  public:
    Lightmap();
    ~Lightmap();

    void start_draw(const Color &ambient_color);
    void end_draw();
    void do_draw();
    void draw_surface(const DrawingRequest& request);
    void draw_surface_part(const DrawingRequest& request);
    void draw_text(const DrawingRequest& request);
    void draw_gradient(const DrawingRequest& request);
    void draw_filled_rect(const DrawingRequest& request);
    void get_light(const DrawingRequest& request) const;

  private:
    SDL_Surface* screen;
    Uint8 *red_channel;
    Uint8 *blue_channel;
    Uint8 *green_channel;
    int width, height;
    int numerator, denominator;
    int LIGHTMAP_DIV;

    void light_blit(SDL_Surface *src, SDL_Rect *src_rect, int dstx, int dsty);
  };
}

#endif

