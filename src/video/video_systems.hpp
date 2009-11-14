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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#ifndef __RENDER_SYTSTEMS_HPP__
#define __RENDER_SYTSTEMS_HPP__

#include <config.h>

#include <string>
#include <SDL.h>

class Renderer;
class Lightmap;
class Texture;
class Surface;

enum VideoSystem {
  AUTO_VIDEO,
  OPENGL,
  PURE_SDL,
  NUM_SYSTEMS
};

Renderer *new_renderer();
Lightmap *new_lightmap();
Texture *new_texture(SDL_Surface *image);
void *new_surface_data(const Surface &surface);
void free_surface_data(void *surface_data);
VideoSystem get_video_system(const std::string &video);
std::string get_video_string(VideoSystem video);

#endif
