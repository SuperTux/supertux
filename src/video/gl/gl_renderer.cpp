//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//	Updated by GiBy 2013 for SDL2 <giby_the_kid@yahoo.fr>
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

#include "video/gl/gl_renderer.hpp"

#include <iomanip>
#include <iostream>
#include <physfs.h>
#include "SDL.h"

#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "video/drawing_request.hpp"
#include "video/gl/gl_surface_data.hpp"
#include "video/gl/gl_texture.hpp"
#include "video/util.hpp"

#define LIGHTMAP_DIV 5

#ifdef GL_VERSION_ES_CM_1_0
#  define glOrtho glOrthof
#endif

GLRenderer::GLRenderer() :
  window(),
  glcontext(),
  viewport(),
  desktop_size(0, 0),
  fullscreen_active(false),
  last_texture(static_cast<GLuint> (-1))
{
  Renderer::instance_ = this;

  SDL_DisplayMode mode;
  SDL_GetCurrentDisplayMode(0, &mode);
  desktop_size = Size(mode.w, mode.h);

  if(texture_manager != 0)
    texture_manager->save_textures();

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

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  5);

  apply_video_mode();

  // setup opengl state and transform
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Init the projection matrix, viewport and stuff
  apply_config();

  if(texture_manager == 0)
    texture_manager = new TextureManager();
  else
    texture_manager->reload_textures();

#ifndef GL_VERSION_ES_CM_1_0
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    std::ostringstream out;
    out << "GLRenderer: " << glewGetErrorString(err);
    throw std::runtime_error(out.str());
  }
  log_info << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  log_info << "GLEW_ARB_texture_non_power_of_two: " << static_cast<int>(GLEW_ARB_texture_non_power_of_two) << std::endl;
#endif
}

GLRenderer::~GLRenderer()
{
  SDL_GL_DeleteContext(glcontext);
  SDL_DestroyWindow(window);
}

void
GLRenderer::draw_surface(const DrawingRequest& request)
{
  const Surface* surface = (const Surface*) request.request_data;
  if(surface == NULL)
  {
    return;
  }
  GLTexture* gltexture = static_cast<GLTexture*>(surface->get_texture().get());
  if(gltexture == NULL)
  {
    return;
  }
  GLSurfaceData *surface_data = static_cast<GLSurfaceData*>(surface->get_surface_data());
  if(surface_data == NULL)
  {
    return;
  }

  GLuint th = gltexture->get_handle();
  if (th != last_texture) {
    last_texture = th;
    glBindTexture(GL_TEXTURE_2D, th);
  }
  intern_draw(request.pos.x, request.pos.y,
              request.pos.x + surface->get_width(),
              request.pos.y + surface->get_height(),
              surface_data->get_uv_left(),
              surface_data->get_uv_top(),
              surface_data->get_uv_right(),
              surface_data->get_uv_bottom(),
              request.angle,
              request.alpha,
              request.color,
              request.blend,
              request.drawing_effect);
}

void
GLRenderer::draw_surface_part(const DrawingRequest& request)
{
  const SurfacePartRequest* surfacepartrequest
    = (SurfacePartRequest*) request.request_data;
  const Surface* surface = surfacepartrequest->surface;
  boost::shared_ptr<GLTexture> gltexture = boost::dynamic_pointer_cast<GLTexture>(surface->get_texture());
  GLSurfaceData *surface_data = reinterpret_cast<GLSurfaceData *>(surface->get_surface_data());

  float uv_width = surface_data->get_uv_right() - surface_data->get_uv_left();
  float uv_height = surface_data->get_uv_bottom() - surface_data->get_uv_top();

  float uv_left = surface_data->get_uv_left() + (uv_width * surfacepartrequest->source.x) / surface->get_width();
  float uv_top = surface_data->get_uv_top() + (uv_height * surfacepartrequest->source.y) / surface->get_height();
  float uv_right = surface_data->get_uv_left() + (uv_width * (surfacepartrequest->source.x + surfacepartrequest->size.x)) / surface->get_width();
  float uv_bottom = surface_data->get_uv_top() + (uv_height * (surfacepartrequest->source.y + surfacepartrequest->size.y)) / surface->get_height();

  GLuint th = gltexture->get_handle();
  if (th != last_texture) {
    last_texture = th;
    glBindTexture(GL_TEXTURE_2D, th);
  }
  intern_draw(request.pos.x, request.pos.y,
              request.pos.x + surfacepartrequest->size.x,
              request.pos.y + surfacepartrequest->size.y,
              uv_left,
              uv_top,
              uv_right,
              uv_bottom,
              0.0,
              request.alpha,
              request.color,
              Blend(),
              request.drawing_effect);
}

void
GLRenderer::draw_gradient(const DrawingRequest& request)
{
  const GradientRequest* gradientrequest
    = (GradientRequest*) request.request_data;
  const Color& top = gradientrequest->top;
  const Color& bottom = gradientrequest->bottom;

  glDisable(GL_TEXTURE_2D);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  float vertices[] = {
    0, 0,
    float(SCREEN_WIDTH), 0,
    float(SCREEN_WIDTH), float(SCREEN_HEIGHT),
    0, float(SCREEN_HEIGHT)
  };
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  float colors[] = {
    top.red, top.green, top.blue, top.alpha,
    top.red, top.green, top.blue, top.alpha,
    bottom.red, bottom.green, bottom.blue, bottom.alpha,
    bottom.red, bottom.green, bottom.blue, bottom.alpha,
  };
  glColorPointer(4, GL_FLOAT, 0, colors);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glDisableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
}

void
GLRenderer::draw_filled_rect(const DrawingRequest& request)
{
  const FillRectRequest* fillrectrequest
    = (FillRectRequest*) request.request_data;

  glDisable(GL_TEXTURE_2D);
  glColor4f(fillrectrequest->color.red, fillrectrequest->color.green,
            fillrectrequest->color.blue, fillrectrequest->color.alpha);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  if (fillrectrequest->radius != 0.0f)
  {
    // draw round rect
    // Keep radius in the limits, so that we get a circle instead of
    // just graphic junk
    float radius = std::min(fillrectrequest->radius,
                            std::min(fillrectrequest->size.x/2,
                                     fillrectrequest->size.y/2));

    // inner rectangle
    Rectf irect(request.pos.x    + radius,
                request.pos.y    + radius,
                request.pos.x + fillrectrequest->size.x - radius,
                request.pos.y + fillrectrequest->size.y - radius);

    int n = 8;
    int p = 0;
    std::vector<float> vertices((n+1) * 4 * 2);

    for(int i = 0; i <= n; ++i)
    {
      float x = sinf(i * (M_PI/2) / n) * radius;
      float y = cosf(i * (M_PI/2) / n) * radius;

      vertices[p++] = irect.get_left() - x;
      vertices[p++] = irect.get_top()  - y;

      vertices[p++] = irect.get_right() + x;
      vertices[p++] = irect.get_top()   - y;
    }

    for(int i = 0; i <= n; ++i)
    {
      float x = cosf(i * (M_PI/2) / n) * radius;
      float y = sinf(i * (M_PI/2) / n) * radius;

      vertices[p++] = irect.get_left()   - x;
      vertices[p++] = irect.get_bottom() + y;

      vertices[p++] = irect.get_right()  + x;
      vertices[p++] = irect.get_bottom() + y;
    }

    glVertexPointer(2, GL_FLOAT, 0, &*vertices.begin());
    glDrawArrays(GL_TRIANGLE_STRIP, 0,  vertices.size()/2);
  }
  else
  {
    float x = request.pos.x;
    float y = request.pos.y;
    float w = fillrectrequest->size.x;
    float h = fillrectrequest->size.y;

    float vertices[] = {
      x,   y,
      x+w, y,
      x+w, y+h,
      x,   y+h
    };
    glVertexPointer(2, GL_FLOAT, 0, vertices);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
}

void
GLRenderer::draw_inverse_ellipse(const DrawingRequest& request)
{
  const InverseEllipseRequest* ellipse = (InverseEllipseRequest*)request.request_data;

  glDisable(GL_TEXTURE_2D);
  glColor4f(ellipse->color.red,  ellipse->color.green,
            ellipse->color.blue, ellipse->color.alpha);

  float x = request.pos.x;
  float y = request.pos.y;
  float w = ellipse->size.x/2.0f;
  float h = ellipse->size.y/2.0f;

  static const int slices = 16;
  static const int points = (slices+1) * 12;

  float vertices[points * 2];
  int   p = 0;

  // Bottom
  vertices[p++] = SCREEN_WIDTH; vertices[p++] = SCREEN_HEIGHT;
  vertices[p++] = 0;            vertices[p++] = SCREEN_HEIGHT;
  vertices[p++] = x;            vertices[p++] = y+h;

  // Top
  vertices[p++] = SCREEN_WIDTH; vertices[p++] = 0;
  vertices[p++] = 0;            vertices[p++] = 0;
  vertices[p++] = x;            vertices[p++] = y-h;

  // Left
  vertices[p++] = SCREEN_WIDTH; vertices[p++] = 0;
  vertices[p++] = SCREEN_WIDTH; vertices[p++] = SCREEN_HEIGHT;
  vertices[p++] = x+w;          vertices[p++] = y;

  // Right
  vertices[p++] = 0;            vertices[p++] = 0;
  vertices[p++] = 0;            vertices[p++] = SCREEN_HEIGHT;
  vertices[p++] = x-w;          vertices[p++] = y;

  for(int i = 0; i < slices; ++i)
  {
    float ex1 = sinf(M_PI/2 / slices * i) * w;
    float ey1 = cosf(M_PI/2 / slices * i) * h;

    float ex2 = sinf(M_PI/2 / slices * (i+1)) * w;
    float ey2 = cosf(M_PI/2 / slices * (i+1)) * h;

    // Bottom/Right
    vertices[p++] = SCREEN_WIDTH; vertices[p++] = SCREEN_HEIGHT;
    vertices[p++] = x + ex1;      vertices[p++] = y + ey1;
    vertices[p++] = x + ex2;      vertices[p++] = y + ey2;

    // Top/Left
    vertices[p++] = 0;            vertices[p++] = 0;
    vertices[p++] = x - ex1;      vertices[p++] = y - ey1;
    vertices[p++] = x - ex2;      vertices[p++] = y - ey2;

    // Top/Right
    vertices[p++] = SCREEN_WIDTH; vertices[p++] = 0;
    vertices[p++] = x + ex1;      vertices[p++] = y - ey1;
    vertices[p++] = x + ex2;      vertices[p++] = y - ey2;

    // Bottom/Left
    vertices[p++] = 0;            vertices[p++] = SCREEN_HEIGHT;
    vertices[p++] = x - ex1;      vertices[p++] = y + ey1;
    vertices[p++] = x - ex2;      vertices[p++] = y + ey2;
  }

  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, vertices);

  glDrawArrays(GL_TRIANGLES, 0, points);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
}

void
GLRenderer::do_take_screenshot()
{
  // [Christoph] TODO: Yes, this method also takes care of the actual disk I/O. Split it?

  SDL_Surface *shot_surf;
  // create surface to hold screenshot
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  shot_surf = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);
#else
  shot_surf = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
#endif
  if (!shot_surf) {
    log_warning << "Could not create RGB Surface to contain screenshot" << std::endl;
    return;
  }

  // read pixels into array
  char* pixels = new char[3 * SCREEN_WIDTH * SCREEN_HEIGHT];
  if (!pixels) {
    log_warning << "Could not allocate memory to store screenshot" << std::endl;
    SDL_FreeSurface(shot_surf);
    return;
  }
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  // copy array line-by-line
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    char* src = pixels + (3 * SCREEN_WIDTH * (SCREEN_HEIGHT - i - 1));
    if(SDL_MUSTLOCK(shot_surf))
    {
      SDL_LockSurface(shot_surf);
    }
    char* dst = ((char*)shot_surf->pixels) + i * shot_surf->pitch;
    memcpy(dst, src, 3 * SCREEN_WIDTH);
    if(SDL_MUSTLOCK(shot_surf))
    {
      SDL_UnlockSurface(shot_surf);
    }
  }

  // free array
  delete[](pixels);

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
      log_debug << "Wrote screenshot to \"" << fullFilename << "\"" << std::endl;
      SDL_FreeSurface(shot_surf);
      return;
    }
  }
  log_warning << "Did not save screenshot, because all files up to \"" << fullFilename << "\" already existed" << std::endl;
  SDL_FreeSurface(shot_surf);
}

void
GLRenderer::flip()
{
  assert_gl("drawing");
  SDL_GL_SwapWindow(window);
}

void
GLRenderer::resize(int w, int h)
{
  g_config->window_size = Size(w, h);

  apply_config();
}

void
GLRenderer::apply_config()
{
  apply_video_mode();

  Size target_size = g_config->use_fullscreen ?
    ((g_config->fullscreen_size == Size(0, 0)) ? desktop_size : g_config->fullscreen_size) :
    g_config->window_size;

  float pixel_aspect_ratio = 1.0f;
  if (g_config->aspect_size != Size(0, 0))
  {
    pixel_aspect_ratio = calculate_pixel_aspect_ratio(desktop_size,
                                                      g_config->aspect_size);
  }
  else if (g_config->use_fullscreen)
  {
    pixel_aspect_ratio = calculate_pixel_aspect_ratio(desktop_size,
                                                      target_size);
  }

  Size max_size(1280, 800);
  Size min_size(640, 480);

  Vector scale;
  Size logical_size;
  calculate_viewport(min_size, max_size, target_size,
                     pixel_aspect_ratio,
                     g_config->magnification,
                     scale,
                     logical_size,
                     viewport);

  SCREEN_WIDTH = logical_size.width;
  SCREEN_HEIGHT = logical_size.height;

  if (viewport.x != 0 || viewport.y != 0)
  {
    // Clear both buffers so that we get a clean black border without junk
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(window);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(window);
  }

  glViewport(viewport.x, viewport.y, viewport.w, viewport.h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, 0);
  check_gl_error("Setting up view matrices");
}

void
GLRenderer::apply_video_mode()
{
  if (window)
  {
    if (!g_config->use_fullscreen)
    {
      SDL_SetWindowFullscreen(window, 0);
    }
    else
    {
      if (g_config->fullscreen_size.width == 0 &&
          g_config->fullscreen_size.height == 0)
      {
        if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
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

        if (SDL_SetWindowDisplayMode(window, &mode) != 0)
        {
          log_warning << "failed to set display mode: "
                      << mode.w << "x" << mode.h << "@" << mode.refresh_rate << ": "
                      << SDL_GetError() << std::endl;
        }
        else
        {
          if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) != 0)
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
  else
  {
    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    Size size;
    if (g_config->use_fullscreen)
    {
      if (g_config->fullscreen_size == Size(0, 0))
      {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        size = desktop_size;
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

    window = SDL_CreateWindow("SuperTux",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              size.width, size.height,
                              flags);
    if (!window)
    {
      std::ostringstream msg;
      msg << "Couldn't set video mode " << size.width << "x" << size.height << ": " << SDL_GetError();
      throw std::runtime_error(msg.str());
    }
    else
    {
      glcontext = SDL_GL_CreateContext(window);

      SCREEN_WIDTH = size.width;
      SCREEN_HEIGHT = size.height;

      fullscreen_active = g_config->use_fullscreen;
    }
  }
}

Vector
GLRenderer::to_logical(int physical_x, int physical_y)
{
  return Vector(static_cast<float>(physical_x - viewport.x) * SCREEN_WIDTH / viewport.w,
                static_cast<float>(physical_y - viewport.y) * SCREEN_HEIGHT / viewport.h);
}

void
GLRenderer::set_gamma(float gamma)
{
  Uint16 ramp[256];
  SDL_CalculateGammaRamp(gamma, ramp);
  SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
}

/* EOF */
