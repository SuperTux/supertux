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

#include "launcher/video_system.hpp"

#include <sstream>

#include "math/rect.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"
#include "video/renderer.hpp"
#include "video/sdl/sdl_screen_renderer.hpp"
#include "video/sdl/sdl_texture.hpp"
#include "video/sdl/sdl_texture_renderer.hpp"
#include "video/sdl_surface.hpp"
#include "video/texture_manager.hpp"

LauncherVideoSystem::LauncherVideoSystem() :
  m_sdl_renderer(nullptr, &SDL_DestroyRenderer),
  m_viewport(),
  m_renderer(),
  m_lightmap(),
  m_texture_manager()
{
  create_window();

  m_renderer.reset(new SDLScreenRenderer(*this, m_sdl_renderer.get()));
  m_texture_manager.reset(new TextureManager);

  apply_config();
}

LauncherVideoSystem::~LauncherVideoSystem()
{
}

std::string
LauncherVideoSystem::get_name() const
{
  SDL_version version;
  SDL_GetVersion(&version);
  std::ostringstream out;
  out << "(Launcher) SDL "
      << static_cast<int>(version.major)
      << "." << static_cast<int>(version.minor)
      << "." << static_cast<int>(version.patch);
  return out.str();
}

void
LauncherVideoSystem::create_window()
{
  log_info << "Creating LauncherVideoSystem" << std::endl;

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

  create_advanced_sdl_window("SuperTux Launcher",
                             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             640, 400, SDL_WINDOW_BORDERLESS);

  m_sdl_renderer.reset(SDL_CreateRenderer(m_sdl_window.get(), -1, 0));
  if (!m_sdl_renderer)
  {
    std::stringstream msg;
    msg << "Couldn't create SDL_Renderer: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
}

void
LauncherVideoSystem::apply_config()
{
  // FIXME: Ugly hack to bypass the fullscreen setting
  bool was_fullscreen = g_config->use_fullscreen;
  g_config->use_fullscreen = false;
  apply_video_mode();
  g_config->use_fullscreen = was_fullscreen;

  m_viewport = Viewport::from_size(Size(640, 400), m_desktop_size);

  m_lightmap.reset(new SDLTextureRenderer(*this, m_sdl_renderer.get(), m_viewport.get_screen_size(), 5));
}

Renderer&
LauncherVideoSystem::get_renderer() const
{
  return *m_renderer;
}

Renderer&
LauncherVideoSystem::get_lightmap() const
{
  return *m_lightmap;
}

TexturePtr
LauncherVideoSystem::new_texture(const SDL_Surface& image, const Sampler& sampler)
{
  return TexturePtr(new SDLTexture(image, sampler));
}

void
LauncherVideoSystem::set_vsync(int mode)
{
  log_warning << "Setting vsync not supported by SDL renderer" << std::endl;
}

int
LauncherVideoSystem::get_vsync() const
{
  return 0;
}

void
LauncherVideoSystem::flip()
{
  m_renderer->flip();
}

SDLSurfacePtr
LauncherVideoSystem::make_screenshot()
{
  int width;
  int height;
  if (SDL_GetRendererOutputSize(m_renderer->get_sdl_renderer(), &width, &height) != 0)
  {
    log_warning << "SDL_GetRenderOutputSize failed: " << SDL_GetError() << std::endl;
    return {};
  }
  else
  {
    SDLSurfacePtr surface = SDLSurface::create_rgba(width, height);

    SDL_LockSurface(surface.get());
    int ret = SDL_RenderReadPixels(m_renderer->get_sdl_renderer(), nullptr,
                                   SDL_PIXELFORMAT_ABGR8888,
                                   surface->pixels,
                                   surface->pitch);
    SDL_UnlockSurface(surface.get());

    if (ret != 0)
    {
      log_warning << "SDL_RenderReadPixels failed: " << SDL_GetError() << std::endl;
      return {};
    }
    else
    {
      return surface;
    }
  }
}

/* EOF */
