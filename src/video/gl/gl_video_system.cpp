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

#ifdef USE_GLBINDING
#  include <glbinding/Binding.h>
#  include <glbinding/ContextInfo.h>
#  include <glbinding/gl/extension.h>
#  include <glbinding/callbacks.h>
#endif

#include <iomanip>
#include <physfs.h>

#include "math/rect.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"
#include "video/gl/gl_lightmap.hpp"
#include "video/gl/gl_renderer.hpp"
#include "video/gl/gl_texture.hpp"

GLVideoSystem::GLVideoSystem() :
  m_texture_manager(),
  m_renderer(),
  m_lightmap(),
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
  m_renderer.reset(new GLRenderer(*this));
  m_lightmap.reset(new GLLightmap(*this));

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

// #ifdef HAVE_OPENGLES2
//   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
//   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
//   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
// #else
//   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
//   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
// #endif

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

  if(g_config->try_vsync) {
    /* we want vsync for smooth scrolling */
    if (SDL_GL_SetSwapInterval(-1) != 0)
    {
      log_info << "no support for late swap tearing vsync: " << SDL_GetError() << std::endl;
      if (SDL_GL_SetSwapInterval(1))
      {
        log_info << "no support for vsync: " << SDL_GetError() << std::endl;
      }
    }
  }

#ifdef USE_GLBINDING
  glbinding::Binding::initialize();

#ifdef USE_GLBINDING_DEBUG_OUTPUT
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
#endif

  static auto extensions = glbinding::ContextInfo::extensions();
  log_info << "Using glbinding" << std::endl;
  log_info << "ARB_texture_non_power_of_two: " << static_cast<int>(extensions.find(GLextension::GL_ARB_texture_non_power_of_two) != extensions.end()) << std::endl;
#endif

#ifndef GL_VERSION_ES_CM_1_0
#ifndef USE_GLBINDING
  GLenum err = glewInit();
#ifdef GLEW_ERROR_NO_GLX_DISPLAY
  if (GLEW_ERROR_NO_GLX_DISPLAY == err)
  {
    log_info << "GLEW couldn't open GLX display" << std::endl;
  }
  else
#endif
    if (GLEW_OK != err)
    {
      std::ostringstream out;
      out << "GLRenderer: " << glewGetErrorString(err);
      throw std::runtime_error(out.str());
    }

  log_info << "OpenGL 3.3: " << GLEW_VERSION_3_3 << std::endl;
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

  if (m_viewport.needs_clear_screen())
  {
    // Clear both buffers so that we get a clean black border without junk
    m_renderer->clear(Color::BLACK);
    flip();
    m_renderer->clear(Color::BLACK);
    flip();
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

Lightmap&
GLVideoSystem::get_lightmap() const
{
  return *m_lightmap;
}

TexturePtr
GLVideoSystem::new_texture(SDL_Surface* image)
{
  return TexturePtr(new GLTexture(image));
}

void
GLVideoSystem::flip()
{
  assert_gl("flip");
  SDL_GL_SwapWindow(m_window);
}

void
GLVideoSystem::on_resize(int w, int h)
{
  g_config->window_size = Size(w, h);

  apply_config();

  m_lightmap.reset(new GLLightmap(*this));
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
GLVideoSystem::set_icon(SDL_Surface* icon)
{
  SDL_SetWindowIcon(m_window, icon);
}

void
GLVideoSystem::do_take_screenshot()
{
  // [Christoph] TODO: Yes, this method also takes care of the actual disk I/O. Split it?

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  const int& viewport_x = viewport[0];
  const int& viewport_y = viewport[1];
  const int& viewport_width = viewport[2];
  const int& viewport_height = viewport[3];

  SDL_Surface *shot_surf;
  // create surface to hold screenshot
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  shot_surf = SDL_CreateRGBSurface(0, viewport_width, viewport_height, 24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);
#else
  shot_surf = SDL_CreateRGBSurface(0, viewport_width, viewport_height, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
#endif
  if (!shot_surf) {
    log_warning << "Could not create RGB Surface to contain screenshot" << std::endl;
    return;
  }

  // read pixels into array
  std::vector<char> pixels(3 * viewport_width * viewport_height);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(viewport_x, viewport_y, viewport_width, viewport_height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

  // copy array line-by-line
  for (int i = 0; i < viewport_height; i++) {
    char* src = &pixels[3 * viewport_width * (viewport_height - i - 1)];
    if(SDL_MUSTLOCK(shot_surf))
    {
      SDL_LockSurface(shot_surf);
    }
    char* dst = (static_cast<char*>(shot_surf->pixels)) + i * shot_surf->pitch;
    memcpy(dst, src, 3 * viewport_width);
    if(SDL_MUSTLOCK(shot_surf))
    {
      SDL_UnlockSurface(shot_surf);
    }
  }

  // save screenshot
  static const std::string writeDir = PHYSFS_getWriteDir();
  static const std::string dirSep = PHYSFS_getDirSeparator();
  static const std::string baseName = "screenshot";
  static const std::string fileExt = ".bmp";
  std::string fullFilename;
  for (int num = 0; num < 1000; num++) {
    std::ostringstream oss;
    oss << baseName;
    oss << std::setw(3) << std::setfill('0') << num;
    oss << fileExt;
    std::string fileName = oss.str();
    fullFilename = writeDir + dirSep + fileName;
    if (!PHYSFS_exists(fileName.c_str())) {
      SDL_SaveBMP(shot_surf, fullFilename.c_str());
      log_info << "Wrote screenshot to \"" << fullFilename << "\"" << std::endl;
      SDL_FreeSurface(shot_surf);
      return;
    }
  }
  log_warning << "Did not save screenshot, because all files up to \"" << fullFilename << "\" already existed" << std::endl;
  SDL_FreeSurface(shot_surf);
}

Size
GLVideoSystem::get_window_size() const
{
  Size size;
  SDL_GetWindowSize(m_window, &size.width, &size.height);
  return size;
}

/* EOF */
