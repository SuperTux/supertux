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

#include "video/sdl/sdl_video_system.hpp"

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

SDLVideoSystem::SDLVideoSystem() :
  m_sdl_window(),
  m_sdl_renderer(),
  m_desktop_size(),
  m_viewport(),
  m_renderer(),
  m_lightmap(),
  m_texture_manager()
{
  SDL_DisplayMode mode;
  if (SDL_GetDesktopDisplayMode(0, &mode) != 0)
  {
    log_warning << "Couldn't get desktop display mode: " << SDL_GetError() << std::endl;
  }
  else
  {
    m_desktop_size = Size(mode.w, mode.h);
  }

  log_info << "Creating SDLVideoSystem" << std::endl;
  int width  = g_config->window_size.width;
  int height = g_config->window_size.height;

  int flags = SDL_WINDOW_RESIZABLE;
  if(g_config->use_fullscreen)
  {
    if (g_config->fullscreen_size == Size(0, 0))
    {
      flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
      width = g_config->window_size.width;
      height = g_config->window_size.height;
    }
    else
    {
      flags |= SDL_WINDOW_FULLSCREEN;
      width  = g_config->fullscreen_size.width;
      height = g_config->fullscreen_size.height;
    }
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

  int ret = SDL_CreateWindowAndRenderer(width, height, flags, &m_sdl_window, &m_sdl_renderer);
  if(ret != 0)
  {
    std::stringstream msg;
    msg << "Couldn't set video mode (" << width << "x" << height
        << "): " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  g_config->window_size = Size(width, height);

  m_renderer.reset(new SDLScreenRenderer(*this, m_sdl_renderer));
  m_texture_manager.reset(new TextureManager);

  apply_config();
}

SDLVideoSystem::~SDLVideoSystem()
{
  SDL_DestroyRenderer(m_sdl_renderer);
  SDL_DestroyWindow(m_sdl_window);
}

void
SDLVideoSystem::apply_config()
{
  apply_video_mode();

  { // apply_viewport
    Size target_size = (g_config->use_fullscreen && g_config->fullscreen_size != Size(0, 0)) ?
      g_config->fullscreen_size :
      g_config->window_size;

    m_viewport = Viewport::from_size(target_size, m_desktop_size);
  }

  m_lightmap.reset(new SDLTextureRenderer(*this, m_sdl_renderer, m_viewport.get_screen_size(), 5));
}

void
SDLVideoSystem::apply_video_mode()
{
  if (!g_config->use_fullscreen)
  {
    SDL_SetWindowFullscreen(m_sdl_window, 0);
  }
  else
  {
    if (g_config->fullscreen_size.width == 0 &&
        g_config->fullscreen_size.height == 0)
    {
      if (SDL_SetWindowFullscreen(m_sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
      {
        log_warning << "failed to switch to desktop fullscreen mode: "
                    << SDL_GetError() << std::endl;
      }
      else
      {
        log_info << "switched to desktop fullscreen mode" << std::endl;
      }
    }
    else
    {
      SDL_DisplayMode mode;
      mode.format = SDL_PIXELFORMAT_RGB888;
      mode.w = g_config->fullscreen_size.width;
      mode.h = g_config->fullscreen_size.height;
      mode.refresh_rate = g_config->fullscreen_refresh_rate;
      mode.driverdata = 0;

      if (SDL_SetWindowDisplayMode(m_sdl_window, &mode) != 0)
      {
        log_warning << "failed to set display mode: "
                    << mode.w << "x" << mode.h << "@" << mode.refresh_rate << ": "
                    << SDL_GetError() << std::endl;
      }
      else
      {
        if (SDL_SetWindowFullscreen(m_sdl_window, SDL_WINDOW_FULLSCREEN) != 0)
        {
          log_warning << "failed to switch to fullscreen mode: "
                      << mode.w << "x" << mode.h << "@" << mode.refresh_rate << ": "
                      << SDL_GetError() << std::endl;
        }
        else
        {
          log_info << "switched to fullscreen mode: "
                   << mode.w << "x" << mode.h << "@" << mode.refresh_rate << std::endl;
        }
      }
    }
  }
}

Renderer&
SDLVideoSystem::get_renderer() const
{
  return *m_renderer;
}

Renderer&
SDLVideoSystem::get_lightmap() const
{
  return *m_lightmap;
}

TexturePtr
SDLVideoSystem::new_texture(const SDL_Surface& image, const Sampler& sampler)
{
  return TexturePtr(new SDLTexture(image, sampler));
}

void
SDLVideoSystem::on_resize(int w, int h)
{
  g_config->window_size = Size(w, h);
  apply_config();
}

void
SDLVideoSystem::set_vsync(int mode)
{
  log_warning << "Setting vsync not supported by SDL renderer" << std::endl;
}

int
SDLVideoSystem::get_vsync() const
{
  return 0;
}

void
SDLVideoSystem::set_gamma(float gamma)
{
  Uint16 ramp[256];
  SDL_CalculateGammaRamp(gamma, ramp);
  SDL_SetWindowGammaRamp(m_sdl_window, ramp, ramp, ramp);
}

void
SDLVideoSystem::set_title(const std::string& title)
{
  SDL_SetWindowTitle(m_sdl_window, title.c_str());
}

void
SDLVideoSystem::set_icon(const SDL_Surface& icon)
{
  SDL_SetWindowIcon(m_sdl_window, const_cast<SDL_Surface*>(&icon));
}

void
SDLVideoSystem::flip()
{
  m_renderer->flip();
}

SDLSurfacePtr
SDLVideoSystem::make_screenshot()
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
    int ret = SDL_RenderReadPixels(m_renderer->get_sdl_renderer(), NULL,
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
