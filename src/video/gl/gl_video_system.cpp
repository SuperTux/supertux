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

#include "math/rect.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"
#include "video/gl/gl20_context.hpp"
#include "video/gl/gl33core_context.hpp"
#include "video/gl/gl_context.hpp"
#include "video/gl/gl_program.hpp"
#include "video/gl/gl_screen_renderer.hpp"
#include "video/gl/gl_texture.hpp"
#include "video/gl/gl_texture_renderer.hpp"
#include "video/gl/gl_texture_renderer.hpp"
#include "video/gl/gl_vertex_arrays.hpp"
#include "video/glutil.hpp"
#include "video/sdl_surface.hpp"
#include "video/texture_manager.hpp"

#ifdef USE_GLBINDING
#  include <glbinding/Binding.h>
#  include <glbinding/ContextInfo.h>
#  include <glbinding/gl/extension.h>
#  include <glbinding/callbacks.h>
#endif

GLVideoSystem::GLVideoSystem(bool use_opengl33core) :
  m_use_opengl33core(use_opengl33core),
  m_texture_manager(),
  m_renderer(),
  m_lightmap(),
  m_back_renderer(),
  m_context(),
  m_window(),
  m_glcontext(),
  m_desktop_size(),
  m_viewport()
{
  SDL_DisplayMode mode;
  SDL_GetCurrentDisplayMode(0, &mode);
  m_desktop_size = Size(mode.w, mode.h);

  create_window();

  m_texture_manager.reset(new TextureManager);
  m_renderer.reset(new GLScreenRenderer(*this));

#if defined(USE_OPENGLES2)
  m_context.reset(new GL33CoreContext(*this));
#elif defined(USE_OPENGLES1)
  m_context.reset(new GL20Context);
#else
  if (use_opengl33core)
  {
    m_context.reset(new GL33CoreContext(*this));
  }
  else
  {
    m_context.reset(new GL20Context);
  }
#endif

  apply_config();
}

GLVideoSystem::~GLVideoSystem()
{
  SDL_GL_DeleteContext(m_glcontext);
  SDL_DestroyWindow(m_window);
}

void
GLVideoSystem::create_window()
{
  int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
  Size size;
  if (g_config->use_fullscreen)
  {
    if (g_config->fullscreen_size == Size(0, 0))
    {
      flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
      size = m_desktop_size;
    }
    else
    {
      flags |= SDL_WINDOW_FULLSCREEN;
      size.width  = g_config->fullscreen_size.width;
      size.height = g_config->fullscreen_size.height;
    }
  }
  else
  {
    size = g_config->window_size;
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // FIXME: why are we requesting a 16bit context?
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  5);

#if defined(USE_OPENGLES2)
  log_info << "Requesting OpenGLES2 context" << std::endl;
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
#elif defined(USE_OPENGLES1)
  log_info << "Requesting OpenGLES1 context" << std::endl;
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
  if (m_use_opengl33core)
  {
    log_info << "Requesting OpenGL 3.3 Core context" << std::endl;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  }
  else
  {
    log_info << "Requesting OpenGL 2.0 context" << std::endl;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0); // this only goes to 0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
  }
#endif

  m_window = SDL_CreateWindow("SuperTux",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              size.width, size.height,
                              flags);
  if (!m_window)
  {
    std::ostringstream msg;
    msg << "Couldn't set video mode " << size.width << "x" << size.height << ": " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  m_glcontext = SDL_GL_CreateContext(m_window);

  if (g_config->try_vsync) {
    // we want vsync for smooth scrolling
    if (SDL_GL_SetSwapInterval(-1) != 0)
    {
      log_info << "no support for late swap tearing vsync: " << SDL_GetError() << std::endl;
      if (SDL_GL_SetSwapInterval(1))
      {
        log_info << "no support for vsync: " << SDL_GetError() << std::endl;
      }
    }
  }

#if defined(USE_OPENGLES2)
  // nothing to do
#elif defined(USE_OPENGLES1)
  // nothing to do
#else
#  ifdef USE_GLBINDING
  glbinding::Binding::initialize();

#    ifdef USE_GLBINDING_DEBUG_OUTPUT
  glbinding::setCallbackMask(glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue);

  glbinding::setAfterCallback([](const glbinding::FunctionCall & call) {
      std::cout << call.function.name() << "(";

      for (unsigned i = 0; i < call.parameters.size(); ++i)
      {
        std::cout << call.parameters[i]->asString();
        if (i < call.parameters.size() - 1)
          std::cout << ", ";
      }

      std::cout << ")";

      if (call.returnValue)
      {
        std::cout << " -> " << call.returnValue->asString();
      }

      std::cout << std::endl;
    });
#    endif
  static auto extensions = glbinding::ContextInfo::extensions();
  log_info << "Using glbinding" << std::endl;
  log_info << "ARB_texture_non_power_of_two: " << static_cast<int>(extensions.find(GLextension::GL_ARB_texture_non_power_of_two) != extensions.end()) << std::endl;
#  else
  GLenum err = glewInit();
#    ifdef GLEW_ERROR_NO_GLX_DISPLAY
  // Glew can't open glx display when it's running on wayland session
  // and thus returns an error. But glXGetProcAddress is fully usable
  // on wayland, so we can just ignore the "no glx display" error.
  //
  // Note that GLEW_ERROR_NO_GLX_DISPLAY needs glew >= 2.1. Older
  // versions assume that glx display is always available and will
  // just crash.
  if (GLEW_ERROR_NO_GLX_DISPLAY == err)
  {
    log_info << "GLEW couldn't open GLX display" << std::endl;
  }
  else
#    endif
    if (GLEW_OK != err)
    {
      std::ostringstream out;
      out << "GLVideoSystem: GlewError: " << glewGetErrorString(err);
      throw std::runtime_error(out.str());
    }

  // log_info << "OpenGL 3.3: " << GLEW_VERSION_3_3 << std::endl;
  log_info << "OpenGL: " << glGetString(GL_VERSION) << std::endl;
  log_info << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  log_info << "GLEW_ARB_texture_non_power_of_two: " << static_cast<int>(GLEW_ARB_texture_non_power_of_two) << std::endl;
#  endif
#endif
}

void
GLVideoSystem::apply_config()
{
  apply_video_mode();

  Size target_size = g_config->use_fullscreen ?
    ((g_config->fullscreen_size == Size(0, 0)) ? m_desktop_size : g_config->fullscreen_size) :
    g_config->window_size;

  m_viewport = Viewport::from_size(target_size, m_desktop_size);

  m_lightmap.reset(new GLTextureRenderer(*this, m_viewport.get_screen_size(), 5));
  if (m_use_opengl33core)
  {
    m_back_renderer.reset(new GLTextureRenderer(*this, m_viewport.get_screen_size(), 1));
  }
}

void
GLVideoSystem::apply_video_mode()
{
  if (!g_config->use_fullscreen)
  {
    SDL_SetWindowFullscreen(m_window, 0);
  }
  else
  {
    if (g_config->fullscreen_size.width == 0 &&
        g_config->fullscreen_size.height == 0)
    {
      if (SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
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

      if (SDL_SetWindowDisplayMode(m_window, &mode) != 0)
      {
        log_warning << "failed to set display mode: "
                    << mode.w << "x" << mode.h << "@" << mode.refresh_rate << ": "
                    << SDL_GetError() << std::endl;
      }
      else
      {
        if (SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN) != 0)
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
GLVideoSystem::get_renderer() const
{
  return *m_renderer;
}

Renderer&
GLVideoSystem::get_lightmap() const
{
  return *m_lightmap;
}

Renderer*
GLVideoSystem::get_back_renderer() const
{
  return m_back_renderer.get();
}

TexturePtr
GLVideoSystem::new_texture(const SDL_Surface& image, const Sampler& sampler)
{
  return TexturePtr(new GLTexture(image, sampler));
}

void
GLVideoSystem::flip()
{
  assert_gl();
  SDL_GL_SwapWindow(m_window);
}

void
GLVideoSystem::on_resize(int w, int h)
{
  g_config->window_size = Size(w, h);

  apply_config();
}

void
GLVideoSystem::set_vsync(int mode)
{
  if (SDL_GL_SetSwapInterval(mode) < 0)
  {
    log_warning << "Setting vsync mode failed: " << SDL_GetError() << std::endl;
  }
  else
  {
    log_info << "Setting vsync mode to " << mode << std::endl;
  }
}

int
GLVideoSystem::get_vsync() const
{
  return SDL_GL_GetSwapInterval();
}

void
GLVideoSystem::set_gamma(float gamma)
{
  Uint16 ramp[256];
  SDL_CalculateGammaRamp(gamma, ramp);
  SDL_SetWindowGammaRamp(m_window, ramp, ramp, ramp);
}

void
GLVideoSystem::set_title(const std::string& title)
{
  SDL_SetWindowTitle(m_window, title.c_str());
}

void
GLVideoSystem::set_icon(const SDL_Surface& icon)
{
  SDL_SetWindowIcon(m_window, const_cast<SDL_Surface*>(&icon));
}

Size
GLVideoSystem::get_window_size() const
{
  Size size;
  SDL_GetWindowSize(m_window, &size.width, &size.height);
  return size;
}

SDLSurfacePtr
GLVideoSystem::make_screenshot()
{
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  const int& viewport_x = viewport[0];
  const int& viewport_y = viewport[1];
  const int& viewport_width = viewport[2];
  const int& viewport_height = viewport[3];

  SDLSurfacePtr surface = SDLSurface::create_rgb(viewport_width, viewport_height);

  std::vector<char> pixels(3 * viewport_width * viewport_height);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(viewport_x, viewport_y, viewport_width, viewport_height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

  SDL_LockSurface(surface.get());
  for (int i = 0; i < viewport_height; i++)
  {
    char* src = &pixels[3 * viewport_width * (viewport_height - i - 1)];
    char* dst = (static_cast<char*>(surface->pixels)) + i * surface->pitch;
    memcpy(dst, src, 3 * viewport_width);
  }
  SDL_UnlockSurface(surface.get());

  return surface;
}

/* EOF */
