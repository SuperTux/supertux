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

#include <iomanip>
#include <physfs.h>

#include "math/rect.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"
#include "video/gl/gl_lightmap.hpp"
#include "video/gl/gl_renderer.hpp"
#include "video/gl/gl_surface_data.hpp"
#include "video/gl/gl_texture.hpp"

GLVideoSystem::GLVideoSystem() :
  m_texture_manager(new TextureManager),
  m_renderer(new GLRenderer),
  m_lightmap(new GLLightmap)
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

void
GLVideoSystem::set_gamma(float gamma)
{
  Uint16 ramp[256];
  SDL_CalculateGammaRamp(gamma, ramp);
  SDL_SetWindowGammaRamp(m_renderer->get_window(), ramp, ramp, ramp);
}

void
GLVideoSystem::set_title(const std::string& title)
{
  SDL_SetWindowTitle(m_renderer->get_window(), title.c_str());
}

void
GLVideoSystem::set_icon(SDL_Surface* icon)
{
  SDL_SetWindowIcon(m_renderer->get_window(), icon);
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
    char* dst = ((char*)shot_surf->pixels) + i * shot_surf->pitch;
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

void
GLVideoSystem::set_clip_rect(const Rect& rect)
{
  int win_w;
  int win_h;
  SDL_GetWindowSize(m_renderer->get_window(), &win_w, &win_h);

  glScissor(win_w * rect.left / SCREEN_WIDTH,
            win_h - (win_h * rect.bottom / SCREEN_HEIGHT),
            win_w * rect.get_width() / SCREEN_WIDTH,
            win_h * rect.get_height() / SCREEN_HEIGHT);
  glEnable(GL_SCISSOR_TEST);
}

void
GLVideoSystem::clear_clip_rect()
{
  glDisable(GL_SCISSOR_TEST);
}

/* EOF */
