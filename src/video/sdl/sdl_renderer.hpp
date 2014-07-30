//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SUPERTUX_VIDEO_SDL_RENDERER_HPP
#define HEADER_SUPERTUX_VIDEO_SDL_RENDERER_HPP

#include "SDL.h"
#include "video/renderer.hpp"

class SDLRenderer : public Renderer
{
public:
  SDLRenderer();
  ~SDLRenderer();

  void draw_surface(const DrawingRequest& request);
  void draw_surface_part(const DrawingRequest& request);
  void draw_gradient(const DrawingRequest& request);
  void draw_filled_rect(const DrawingRequest& request);
  void draw_inverse_ellipse(const DrawingRequest& request);
  void do_take_screenshot();
  void flip();
  void resize(int w, int h);
  void apply_config() {}
  void set_gamma(float gamma);

  SDL_Renderer* get_sdl_renderer() const { return renderer; };

private:
  SDL_Window* window;
  SDL_Renderer* renderer;

private:
  SDLRenderer(const SDLRenderer&);
  SDLRenderer& operator=(const SDLRenderer&);
};

#endif

/* EOF */
