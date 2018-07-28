//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_VIDEO_SDL_PAINTER_HPP
#define HEADER_SUPERTUX_VIDEO_SDL_PAINTER_HPP

struct DrawingRequest;
struct SDL_Renderer;

class SDLPainter
{
public:
  SDLPainter() {}

  void draw_surface(SDL_Renderer* renderer, const DrawingRequest& request);
  void draw_surface_part(SDL_Renderer* renderer, const DrawingRequest& request);
  void draw_gradient(SDL_Renderer* renderer, const DrawingRequest& request);
  void draw_filled_rect(SDL_Renderer* renderer, const DrawingRequest& request);
  void draw_inverse_ellipse(SDL_Renderer* renderer, const DrawingRequest& request);
  void draw_line(SDL_Renderer* renderer, const DrawingRequest& request);
  void draw_triangle(SDL_Renderer* renderer, const DrawingRequest& request);

private:
  SDLPainter(const SDLPainter&);
  SDLPainter& operator=(const SDLPainter&);
};

#endif

/* EOF */
