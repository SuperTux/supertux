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

#ifndef HEADER_SUPERTUX_VIDEO_SDL_LIGHTMAP_HPP
#define HEADER_SUPERTUX_VIDEO_SDL_LIGHTMAP_HPP

#include "video/lightmap.hpp"

#include <SDL.h>
#include <boost/optional.hpp>

class Color;
struct DrawingRequest;
struct SDL_Renderer;
struct SDL_Texture;

class SDLLightmap : public Lightmap
{
public:
  SDLLightmap(SDL_Renderer* renderer);
  ~SDLLightmap();

  virtual void start_draw() override;
  virtual void end_draw() override;

  virtual void draw_surface(const DrawingRequest& request) override;
  virtual void draw_surface_part(const DrawingRequest& request) override;
  virtual void draw_gradient(const DrawingRequest& request) override;
  virtual void draw_filled_rect(const DrawingRequest& request) override;
  virtual void draw_inverse_ellipse(const DrawingRequest& request) override;
  virtual void draw_line(const DrawingRequest& request) override;
  virtual void draw_triangle(const DrawingRequest& request) override;
  virtual void clear(const Color& color) override;

  virtual void set_clip_rect(const Rect& rect) override;
  virtual void clear_clip_rect() override;

  virtual void get_light(const DrawingRequest& request) const override;
  virtual void render() override;

private:
  SDL_Renderer* m_renderer;
  SDL_Texture* m_texture;
  int m_width;
  int m_height;
  int m_LIGHTMAP_DIV;
  boost::optional<SDL_Rect> m_cliprect;

private:
  SDLLightmap(const SDLLightmap&);
  SDLLightmap& operator=(const SDLLightmap&);
};

#endif

/* EOF */
