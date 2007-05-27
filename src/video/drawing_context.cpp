//  $Id$
//
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include <functional>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <SDL_image.h>
#include <GL/gl.h>
#include <sstream>
#include <iomanip>
#include <physfs.h>

#include "drawing_context.hpp"
#include "surface.hpp"
#include "font.hpp"
#include "main.hpp"
#include "gameconfig.hpp"
#include "glutil.hpp"
#include "texture.hpp"
#include "texture_manager.hpp"
#include "obstack/obstackpp.hpp"
#define LIGHTMAP_DIV 5

enum RequestType
{
  SURFACE, SURFACE_PART, TEXT, GRADIENT, FILLRECT, LIGHTMAPREQUEST, GETLIGHT
};

struct SurfacePartRequest
{
  const Surface* surface;
  Vector source, size;
};

struct TextRequest
{
  const Font* font;
  std::string text;
  FontAlignment alignment;
};

struct GradientRequest
{
  Color top, bottom;
  Vector size;
};

struct FillRectRequest
{
  Color color;
  Vector size;
};

struct DrawingRequest
{
  RequestType type;
  Vector pos;

  int layer;
  DrawingEffect drawing_effect;
  float alpha;
  Blend blend;
  float angle;
  Color color;

  void* request_data;

  DrawingRequest()
    : angle(0.0f),
      color(1.0f, 1.0f, 1.0f, 1.0f)
  {}

  bool operator<(const DrawingRequest& other) const
  {
    return layer < other.layer;
  }
};

struct GetLightRequest
{
  Color* color_ptr;
};

static inline int next_po2(int val)
{
  int result = 1;
  while(result < val)
    result *= 2;

  return result;
}

DrawingContext::DrawingContext()
  : ambient_color(1.0f, 1.0f, 1.0f, 1.0f), target(NORMAL), screenshot_requested(false)
{
  screen = SDL_GetVideoSurface();

  lightmap_width = screen->w / LIGHTMAP_DIV;
  lightmap_height = screen->h / LIGHTMAP_DIV;
  unsigned int width = next_po2(lightmap_width);
  unsigned int height = next_po2(lightmap_height);

  lightmap = new Texture(width, height, GL_RGB);

  lightmap_uv_right = static_cast<float>(lightmap_width) / static_cast<float>(width);
  lightmap_uv_bottom = static_cast<float>(lightmap_height) / static_cast<float>(height);
  texture_manager->register_texture(lightmap);

  requests = &drawing_requests;

  obstack_init(&obst);
}

DrawingContext::~DrawingContext()
{
  obstack_free(&obst, NULL);

  texture_manager->remove_texture(lightmap);
  delete lightmap;
}

void
DrawingContext::draw_surface(const Surface* surface, const Vector& position,
                             float angle, const Color& color, const Blend& blend,
                             int layer)
{
  assert(surface != 0);

  DrawingRequest* request = new(obst) DrawingRequest();

  request->type = SURFACE;
  request->pos = transform.apply(position);

  if(request->pos.x >= SCREEN_WIDTH || request->pos.y >= SCREEN_HEIGHT
      || request->pos.x + surface->get_width() < 0
      || request->pos.y + surface->get_height() < 0)
    return;

  request->layer = layer;
  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;
  request->angle = angle;
  request->color = color;
  request->blend = blend;

  request->request_data = const_cast<Surface*> (surface);

  requests->push_back(request);
}

void
DrawingContext::draw_surface(const Surface* surface, const Vector& position,
    int layer)
{
  draw_surface(surface, position, 0.0f, Color(1.0f, 1.0f, 1.0f), Blend(), layer);
}

void
DrawingContext::draw_surface_part(const Surface* surface, const Vector& source,
    const Vector& size, const Vector& dest, int layer)
{
  assert(surface != 0);

  DrawingRequest* request = new(obst) DrawingRequest();

  request->type = SURFACE_PART;
  request->pos = transform.apply(dest);
  request->layer = layer;
  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  SurfacePartRequest* surfacepartrequest = new(obst) SurfacePartRequest();
  surfacepartrequest->size = size;
  surfacepartrequest->source = source;
  surfacepartrequest->surface = surface;

  // clip on screen borders
  if(request->pos.x < 0) {
    surfacepartrequest->size.x += request->pos.x;
    if(surfacepartrequest->size.x <= 0)
      return;
    surfacepartrequest->source.x -= request->pos.x;
    request->pos.x = 0;
  }
  if(request->pos.y < 0) {
    surfacepartrequest->size.y += request->pos.y;
    if(surfacepartrequest->size.y <= 0)
      return;
    surfacepartrequest->source.y -= request->pos.y;
    request->pos.y = 0;
  }
  request->request_data = surfacepartrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_text(const Font* font, const std::string& text,
    const Vector& position, FontAlignment alignment, int layer)
{
  DrawingRequest* request = new(obst) DrawingRequest();

  request->type = TEXT;
  request->pos = transform.apply(position);
  request->layer = layer;
  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  TextRequest* textrequest = new(obst) TextRequest();
  textrequest->font = font;
  textrequest->text = text;
  textrequest->alignment = alignment;
  request->request_data = textrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_center_text(const Font* font, const std::string& text,
    const Vector& position, int layer)
{
  draw_text(font, text, Vector(position.x + SCREEN_WIDTH/2, position.y),
      ALIGN_CENTER, layer);
}

void
DrawingContext::draw_gradient(const Color& top, const Color& bottom, int layer)
{
  DrawingRequest* request = new(obst) DrawingRequest();

  request->type = GRADIENT;
  request->pos = Vector(0,0);
  request->layer = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  GradientRequest* gradientrequest = new(obst) GradientRequest();
  gradientrequest->top = top;
  gradientrequest->bottom = bottom;
  request->request_data = gradientrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_filled_rect(const Vector& topleft, const Vector& size,
                                 const Color& color, int layer)
{
  DrawingRequest* request = new(obst) DrawingRequest();

  request->type = FILLRECT;
  request->pos = transform.apply(topleft);
  request->layer = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  FillRectRequest* fillrectrequest = new(obst) FillRectRequest();
  fillrectrequest->size = size;
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * transform.alpha;
  request->request_data = fillrectrequest;

  requests->push_back(request);
}

void
DrawingContext::draw_filled_rect(const Rect& rect, const Color& color,
                                 int layer)
{
  DrawingRequest* request = new(obst) DrawingRequest();

  request->type = FILLRECT;
  request->pos = transform.apply(rect.p1);
  request->layer = layer;

  request->drawing_effect = transform.drawing_effect;
  request->alpha = transform.alpha;

  FillRectRequest* fillrectrequest = new(obst) FillRectRequest;
  fillrectrequest->size = Vector(rect.get_width(), rect.get_height());
  fillrectrequest->color = color;
  fillrectrequest->color.alpha = color.alpha * transform.alpha;
  request->request_data = fillrectrequest;

  requests->push_back(request);
}

void
DrawingContext::get_light(const Vector& position, Color* color)
{
  if( ambient_color.red == 1.0f && ambient_color.green == 1.0f
      && ambient_color.blue  == 1.0f ) {
    *color = Color( 1.0f, 1.0f, 1.0f);
    return;
  }

  DrawingRequest* request = new(obst) DrawingRequest();
  request->type = GETLIGHT;
  request->pos = transform.apply(position);

  //There is no light offscreen.
  if(request->pos.x >= SCREEN_WIDTH || request->pos.y >= SCREEN_HEIGHT
      || request->pos.x < 0 || request->pos.y < 0){
    *color = Color( 0, 0, 0);
    return;
  }

  request->layer = LAYER_GUI; //make sure all get_light requests are handled last.
  GetLightRequest* getlightrequest = new(obst) GetLightRequest();
  getlightrequest->color_ptr = color;
  request->request_data = getlightrequest;
  lightmap_requests.push_back(request);
}

void
DrawingContext::get_light(const DrawingRequest& request) const
{
  const GetLightRequest* getlightrequest 
    = (GetLightRequest*) request.request_data;

  float pixels[3];
  for( int i = 0; i<3; i++)
    pixels[i] = 0.0f; //set to black

  float posX = request.pos.x * lightmap_width / SCREEN_WIDTH;
  float posY = screen->h - request.pos.y * lightmap_height / SCREEN_HEIGHT;
  glReadPixels((GLint) posX, (GLint) posY , 1, 1, GL_RGB, GL_FLOAT, pixels);
    *(getlightrequest->color_ptr) = Color( pixels[0], pixels[1], pixels[2]);
  //printf("get_light %f/%f =>%f/%f r%f g%f b%f\n", request.pos.x, request.pos.y, posX, posY, pixels[0], pixels[1], pixels[2]);
}

void
DrawingContext::draw_surface_part(const DrawingRequest& request) const
{
  const SurfacePartRequest* surfacepartrequest
    = (SurfacePartRequest*) request.request_data;

  surfacepartrequest->surface->draw_part(
      surfacepartrequest->source.x, surfacepartrequest->source.y,
      request.pos.x, request.pos.y,
      surfacepartrequest->size.x, surfacepartrequest->size.y,
      request.alpha, request.drawing_effect);
}

void
DrawingContext::draw_gradient(const DrawingRequest& request) const
{
  const GradientRequest* gradientrequest 
    = (GradientRequest*) request.request_data;
  const Color& top = gradientrequest->top;
  const Color& bottom = gradientrequest->bottom;

  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glColor4f(top.red, top.green, top.blue, top.alpha);
  glVertex2f(0, 0);
  glVertex2f(SCREEN_WIDTH, 0);
  glColor4f(bottom.red, bottom.green, bottom.blue, bottom.alpha);
  glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
  glVertex2f(0, SCREEN_HEIGHT);
  glEnd();
  glEnable(GL_TEXTURE_2D);
  glColor4f(1, 1, 1, 1);
}

void
DrawingContext::draw_text(const DrawingRequest& request) const
{
  const TextRequest* textrequest = (TextRequest*) request.request_data;

  textrequest->font->draw(textrequest->text, request.pos,
      textrequest->alignment, request.drawing_effect, request.alpha);
}

void
DrawingContext::draw_filled_rect(const DrawingRequest& request) const
{
  const FillRectRequest* fillrectrequest
    = (FillRectRequest*) request.request_data;

  float x = request.pos.x;
  float y = request.pos.y;
  float w = fillrectrequest->size.x;
  float h = fillrectrequest->size.y;

  glDisable(GL_TEXTURE_2D);
  glColor4f(fillrectrequest->color.red, fillrectrequest->color.green,
            fillrectrequest->color.blue, fillrectrequest->color.alpha);

  glBegin(GL_QUADS);
  glVertex2f(x, y);
  glVertex2f(x+w, y);
  glVertex2f(x+w, y+h);
  glVertex2f(x, y+h);
  glEnd();
  glEnable(GL_TEXTURE_2D);
  
  glColor4f(1, 1, 1, 1);
}

void
DrawingContext::draw_lightmap(const DrawingRequest& request) const
{
  const Texture* texture = reinterpret_cast<Texture*> (request.request_data);

  // multiple the lightmap with the framebuffer
  glBlendFunc(GL_DST_COLOR, GL_ZERO);

  glBindTexture(GL_TEXTURE_2D, texture->get_handle());
  glBegin(GL_QUADS);

  glTexCoord2f(0, lightmap_uv_bottom);
  glVertex2f(0, 0);

  glTexCoord2f(lightmap_uv_right, lightmap_uv_bottom);
  glVertex2f(SCREEN_WIDTH, 0);

  glTexCoord2f(lightmap_uv_right, 0);
  glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);

  glTexCoord2f(0, 0);
  glVertex2f(0, SCREEN_HEIGHT);

  glEnd();

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
DrawingContext::do_drawing()
{
#ifdef DEBUG
  assert(transformstack.empty());
  assert(target_stack.empty());
#endif
  transformstack.clear();
  target_stack.clear();

  //Use Lightmap if ambient color is not white.
  bool use_lightmap = ( ambient_color.red != 1.0f   || ambient_color.green != 1.0f ||
                        ambient_color.blue  != 1.0f );

  // PART1: create lightmap
  if(use_lightmap) {
    glViewport(0, screen->h - lightmap_height, lightmap_width, lightmap_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor( ambient_color.red, ambient_color.green, ambient_color.blue, 1 );
    glClear(GL_COLOR_BUFFER_BIT);
    handle_drawing_requests(lightmap_requests);
    lightmap_requests.clear();

    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, lightmap->get_handle());
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, screen->h - lightmap_height, lightmap_width, lightmap_height);

    glViewport(0, 0, screen->w, screen->h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);

    // add a lightmap drawing request into the queue
    DrawingRequest* request = new(obst) DrawingRequest();
    request->type = LIGHTMAPREQUEST;
    request->layer = LAYER_HUD - 1;
    request->request_data = lightmap;
    requests->push_back(request);
  }

  //glClear(GL_COLOR_BUFFER_BIT);
  handle_drawing_requests(drawing_requests);
  drawing_requests.clear();
  obstack_free(&obst, NULL);
  obstack_init(&obst);
  assert_gl("drawing");

  // if a screenshot was requested, take one
  if (screenshot_requested) {
    do_take_screenshot();
    screenshot_requested = false;
  }

  SDL_GL_SwapBuffers();
}

class RequestPtrCompare
  :  public std::binary_function<const DrawingRequest*,
                                 const DrawingRequest*, 
                                 bool>
{
public:
  bool operator()(const DrawingRequest* r1, const DrawingRequest* r2) const
  {
    return *r1 < *r2;
  }
};

void
DrawingContext::handle_drawing_requests(DrawingRequests& requests) const
{
  std::stable_sort(requests.begin(), requests.end(), RequestPtrCompare());

  DrawingRequests::const_iterator i;
  for(i = requests.begin(); i != requests.end(); ++i) {
    const DrawingRequest& request = **i;

    switch(request.type) {
      case SURFACE:
      {
        const Surface* surface = (const Surface*) request.request_data;
        if (request.angle == 0.0f &&
            request.color.red == 1.0f && request.color.green == 1.0f  &&
            request.color.blue == 1.0f &&  request.color.alpha == 1.0f) {
          surface->draw(request.pos.x, request.pos.y, request.alpha,
              request.drawing_effect);
        } else {
          surface->draw(request.pos.x, request.pos.y,
              request.alpha, request.angle, request.color,
              request.blend, request.drawing_effect);
        }
        break;
      }
      case SURFACE_PART:
        draw_surface_part(request);
        break;
      case GRADIENT:
        draw_gradient(request);
        break;
      case TEXT:
        draw_text(request);
        break;
      case FILLRECT:
        draw_filled_rect(request);
        break;
      case LIGHTMAPREQUEST:
        draw_lightmap(request);
        break;
      case GETLIGHT:
        get_light(request);
        break;
    }
  }
}

void
DrawingContext::push_transform()
{
  transformstack.push_back(transform);
}

void
DrawingContext::pop_transform()
{
  assert(!transformstack.empty());

  transform = transformstack.back();
  transformstack.pop_back();
}

void
DrawingContext::set_drawing_effect(DrawingEffect effect)
{
  transform.drawing_effect = effect;
}

DrawingEffect
DrawingContext::get_drawing_effect() const
{
  return transform.drawing_effect;
}

void
DrawingContext::set_alpha(float alpha)
{
  transform.alpha = alpha;
}

float
DrawingContext::get_alpha() const
{
  return transform.alpha;
}

void
DrawingContext::push_target()
{
  target_stack.push_back(target);
}

void
DrawingContext::pop_target()
{
  set_target(target_stack.back());
  target_stack.pop_back();
}

void
DrawingContext::set_target(Target target)
{
  this->target = target;
  if(target == LIGHTMAP) {
    requests = &lightmap_requests;
  } else {
    assert(target == NORMAL);
    requests = &drawing_requests;
  }
}

void
DrawingContext::set_ambient_color( Color new_color )
{
  ambient_color = new_color;
}

void 
DrawingContext::do_take_screenshot()
{
  // [Christoph] TODO: Yes, this method also takes care of the actual disk I/O. Split it?

  // create surface to hold screenshot
  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
  SDL_Surface* shot_surf = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);
  #else
  SDL_Surface* shot_surf = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
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
  glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  // copy array line-by-line
  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    char* src = pixels + (3 * SCREEN_WIDTH * (SCREEN_HEIGHT - i - 1));
    char* dst = ((char*)shot_surf->pixels) + i * shot_surf->pitch;
    memcpy(dst, src, 3 * SCREEN_WIDTH);
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
DrawingContext::take_screenshot()
{
  screenshot_requested = true;
}

