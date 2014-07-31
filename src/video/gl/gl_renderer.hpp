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

#ifndef HEADER_SUPERTUX_VIDEO_GL_RENDERER_HPP
#define HEADER_SUPERTUX_VIDEO_GL_RENDERER_HPP

#include "math/size.hpp"
#include "video/drawing_request.hpp"
#include "video/renderer.hpp"

#include "SDL.h"
#include <math.h>

namespace {

inline void intern_draw(float left, float top, float right, float bottom,
                        float uv_left, float uv_top,
                        float uv_right, float uv_bottom,
                        float angle, float alpha,
                        const Color& color,
                        const Blend& blend,
                        DrawingEffect effect)
{
  if(effect & HORIZONTAL_FLIP)
    std::swap(uv_left, uv_right);
 
  if(effect & VERTICAL_FLIP) 
    std::swap(uv_top, uv_bottom);

  glBlendFunc(blend.sfactor, blend.dfactor);
  glColor4f(color.red, color.green, color.blue, color.alpha * alpha);
 
  // unrotated blit
  if (angle == 0.0f) {
    float vertices[] = {
      left, top,
      right, top,
      right, bottom,
      left, bottom,
    };
    glVertexPointer(2, GL_FLOAT, 0, vertices);

    float uvs[] = {
      uv_left, uv_top,
      uv_right, uv_top,
      uv_right, uv_bottom,
      uv_left, uv_bottom,
    };
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  } else {
    // rotated blit
    float center_x = (left + right) / 2;
    float center_y = (top + bottom) / 2;

    float sa = sinf(angle/180.0f*M_PI);
    float ca = cosf(angle/180.0f*M_PI);

    left  -= center_x;
    right -= center_x;

    top    -= center_y;
    bottom -= center_y;

    float vertices[] = {
      left*ca - top*sa + center_x, left*sa + top*ca + center_y,
      right*ca - top*sa + center_x, right*sa + top*ca + center_y,
      right*ca - bottom*sa + center_x, right*sa + bottom*ca + center_y,
      left*ca - bottom*sa + center_x, left*sa + bottom*ca + center_y
    };
    glVertexPointer(2, GL_FLOAT, 0, vertices);

    float uvs[] = {
      uv_left, uv_top,
      uv_right, uv_top,
      uv_right, uv_bottom,
      uv_left, uv_bottom,
    };
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }

  // FIXME: find a better way to restore the blend mode
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

} // namespace



class GLRenderer : public Renderer
{
private:
  SDL_Window* window;
  SDL_GLContext glcontext;
  Size desktop_size;
  Size screen_size;
  bool fullscreen_active;
	
  GLuint last_texture;

public:
  GLRenderer();
  ~GLRenderer();

  void draw_surface(const DrawingRequest& request);
  void draw_surface_part(const DrawingRequest& request);
  void draw_gradient(const DrawingRequest& request);
  void draw_filled_rect(const DrawingRequest& request);
  void draw_inverse_ellipse(const DrawingRequest& request);
  void do_take_screenshot();
  void flip();
  void resize(int w, int h);
  void apply_config();
  void apply_video_mode(const Size& size, bool fullscreen);
  void set_gamma(float gamma);
  SDL_Window* get_window() const { return window; }
};

#endif

/* EOF */
