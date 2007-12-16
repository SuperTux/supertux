//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "Window.hpp"
#include "Texture.hpp"
#include <unison/video/Window.hpp>
#include <unison/video/Surface.hpp>
#include <unison/video/Texture.hpp>
#include <unison/video/Renderers.hpp>
#include <unison/video/sdl/Blitters.hpp>
#include <unison/video/backend/Renderer.hpp>
#include <unison/video/backend/Texture.hpp>

#include <assert.h>

#include "SDL.h"

namespace Unison
{
   namespace Video
   {
      namespace OpenGL
      {
         Window::Window(const Area &size, const Area &logical_size, bool fullscreen) :
            logical_size(logical_size),
            window(0)
         {
            assert(size.x);
            assert(size.y);

            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            //SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
            SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);

            window = SDL_SetVideoMode(size.x, size.y, 0, SDL_OPENGL | (fullscreen ? SDL_FULLSCREEN : 0));
            assert(window);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glEnable(GL_TEXTURE_2D);
            //glEnable(GL_BLEND);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, logical_size.x, logical_size.y, 0, -1.0, 1.0);
            glMatrixMode(GL_MODELVIEW);
            glViewport(0, 0, size.x, size.y);
            glLoadIdentity();
            //glTranslatef(0, 0, 0);
            //glScalef(GLfloat(size.x) / logical_size.x, GLfloat(size.y) / logical_size.y, 1);
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            assert(!glGetError());
         }

         Window::~Window()
         {
         }

         void Window::take_screenshot(const std::string &filename) const
         {
            //Surface surface(logical_size);
            Surface surface(get_size());
            glReadBuffer(GL_FRONT);
            glReadPixels(0, 0, get_size().x, get_size().y, GL_RGBA, GL_UNSIGNED_BYTE, surface.get_pixels());
            //glReadPixels(0, 0, logical_size.x, logical_size.y, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
            surface.v_flip().save(filename);
         }

         void Window::flip()
         {
            SDL_GL_SwapBuffers();
         }

         void Window::set_title(const std::string &title)
         {
            SDL_WM_SetCaption(title.c_str(), title.c_str());
         }

         void Window::set_icon(const Surface &icon)
         {
            SDL_Surface *icon_surface = SDL::Blitters::create_sdl_surface_from(icon);
            assert(icon_surface);
            SDL_WM_SetIcon(icon_surface, 0);
            SDL_FreeSurface(icon_surface);
         }

         Area Window::get_size() const
         {
            return Area(window->w, window->h);
         }

         bool Window::is_fullscreen() const
         {
            return window->flags & SDL_FULLSCREEN;
         }

         void Window::blit(const Surface &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
         {
            Video::Texture texture(src);
            blit(src, dst_pos, src_rect, options);
         }

         void Window::blit(const Video::Texture &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
         {
            Texture *texture = dynamic_cast<Texture *>(Backend::Texture::get_texture(src.get_id()));
            assert(texture);
            assert(window);

            texture->blit_draw_buffer(src_rect, dst_pos, options);
         }

         void Window::fill(const Color &color, const Rect &rect)
         {
            assert(window);

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            glColor4ub(color.red, color.green, color.blue, color.alpha);
            if(rect == Rect())
            {
               glBegin(GL_QUADS);
                  glVertex2i(0, 0);
                  glVertex2i(get_size().x, 0);
                  glVertex2i(get_size().x, get_size().y);
                  glVertex2i(0, get_size().y);
               glEnd();
            }
            else
            {
               glPushMatrix();

               glTranslatef(rect.pos.x, rect.pos.y, 0);

               glBegin(GL_QUADS);
                  glVertex2i(0, 0);
                  glVertex2i(rect.size.x, 0);
                  glVertex2i(rect.size.x, rect.size.y);
                  glVertex2i(0, rect.size.y);
               glEnd();

               glPopMatrix();
            }
            glEnable(GL_TEXTURE_2D);
            //glColor4ub(0xff, 0xff, 0xff, 0xff);
         }

         void Window::fill_blend(const Color &color, const Rect &rect)
         {
            assert(window);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_TEXTURE_2D);
            glColor4ub(color.red, color.green, color.blue, color.alpha);
            if(rect == Rect())
            {
               glBegin(GL_QUADS);
                  glVertex2i(0, 0);
                  glVertex2i(get_size().x, 0);
                  glVertex2i(get_size().x, get_size().y);
                  glVertex2i(0, get_size().y);
               glEnd();
            }
            else
            {
               glPushMatrix();

               glTranslatef(rect.pos.x, rect.pos.y, 0);

               glBegin(GL_QUADS);
                  glVertex2i(0, 0);
                  glVertex2i(rect.size.x, 0);
                  glVertex2i(rect.size.x, rect.size.y);
                  glVertex2i(0, rect.size.y);
               glEnd();

               glPopMatrix();
            }
            glEnable(GL_TEXTURE_2D);
            //glColor4ub(0xff, 0xff, 0xff, 0xff);
         }
      }
   }
}
