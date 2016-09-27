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

#include "video/gl/gl_video_system.hpp"

#include "video/gl/gl_lightmap.hpp"
#include "video/gl/gl_renderer.hpp"
#include "video/gl/gl_surface_data.hpp"
#include "video/gl/gl_texture.hpp"
#include "video/texture_manager.hpp"

GLVideoSystem::GLVideoSystem() :
  m_texture_manager(new TextureManager),
  m_renderer(new GLRenderer),
  m_lightmap(new GLLightmap),
  m_hidden_lightmap(new GLLightmap)
{
}

Renderer&
GLVideoSystem::get_renderer() const
{
  return *m_renderer;
}

Lightmap&
GLVideoSystem::get_lightmap() const
{
  return *m_lightmap;
}

Lightmap&
GLVideoSystem::get_hidden_lightmap() const
{
  return *m_hidden_lightmap;
}

TexturePtr
GLVideoSystem::new_texture(SDL_Surface* image)
{
  return TexturePtr(new GLTexture(image));
}

SurfaceData*
GLVideoSystem::new_surface_data(const Surface& surface)
{
  return new GLSurfaceData(surface);
}

void
GLVideoSystem::free_surface_data(SurfaceData* surface_data)
{
  delete surface_data;
}

void
GLVideoSystem::apply_config()
{
  m_renderer->apply_config();
}

void
GLVideoSystem::resize(int w, int h)
{
  m_renderer->resize(w, h);
  m_lightmap.reset(new GLLightmap);
}

/* EOF */
