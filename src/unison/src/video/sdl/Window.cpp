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

#include <functional>
#include <assert.h>

#include "SDL.h"

namespace Unison
{
   namespace Video
   {
      namespace SDL
      {
         Window::Window(const Area &size, const Area &logical_size, bool fullscreen) :
            scale_numerator(1),
            scale_denominator(1),
            offset(),
            window(0)
         {
            assert(size.x);
            assert(size.y);
            unsigned int xratio = size.x * logical_size.y;
            unsigned int yratio = size.y * logical_size.x;
            if(xratio < yratio)
            {
               scale_numerator = size.x;
               scale_denominator = logical_size.x;
            }
            else
            {
               scale_numerator = size.y;
               scale_denominator = logical_size.y;
            }
            offset = (size - logical_size * scale_numerator / scale_denominator) / 2;
            window = SDL_SetVideoMode(size.x, size.y, 0, SDL_ANYFORMAT | SDL_SWSURFACE | (fullscreen ? SDL_FULLSCREEN : 0));
            assert(window);
            Uint32 black = SDL_MapRGB(window->format, 0, 0, 0);
            SDL_FillRect(window, 0, black);
         }

         Window::~Window()
         {
         }

         void Window::take_screenshot(const std::string &filename) const
         {
            assert(window);
            SDL_SaveBMP(window, filename.c_str());
         }

         void Window::flip()
         {
            assert(window);
            SDL_Flip(window);
         }

         void Window::set_title(const std::string &title)
         {
            SDL_WM_SetCaption(title.c_str(), title.c_str());
         }

         void Window::set_icon(const Surface &icon)
         {
            SDL_Surface *icon_surface = Blitters::create_sdl_surface_from(icon);
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
            assert(src.get_pixels());
            assert(window);

            Surface fragment;
            if(src_rect.pos == Point() && (src_rect.size == Area() || src_rect.size == src.get_size()))
            {
               fragment = src;
            }
            else
            {
               fragment = Surface(src_rect.size);
               Video::Blitters::blit_blend_none(src, src_rect, fragment, Point());
            }
            fragment = fragment.scale(scale_numerator, scale_denominator);
            if(options.h_flip)
            {
               fragment = fragment.h_flip();
            }
            if(options.v_flip)
            {
               fragment = fragment.v_flip();
            }
            SDL_Surface *src_surface = Blitters::create_sdl_surface_from(fragment.modulate(options.color).modulate(options.alpha));
            assert(src_surface);

            Point scaled_dst_pos(dst_pos);
            scaled_dst_pos *= scale_numerator;
            scaled_dst_pos /= scale_denominator;
            scaled_dst_pos += offset;

            Blitters::blit_blend(src_surface, Rect(Point(), fragment.get_size()), window, scaled_dst_pos, options.blend);

            SDL_FreeSurface(src_surface);
         }

         void Window::blit(const Video::Texture &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
         {
            Texture *texture = dynamic_cast<Texture *>(Backend::Texture::get_texture(src.get_id()));
            assert(texture);
            assert(window);

            Rect scaled_src_rect(src_rect);
            /*if(options.h_flip)
            {
               scaled_src_rect.pos.x = src.get_size().x - src_rect.pos.x - src_rect.size.x;
            }
            if(options.v_flip)
            {
               scaled_src_rect.pos.y = src.get_size().y - src_rect.pos.y - src_rect.size.y;
            }*/
            if(dst_pos.x < 0)
            {
               if(scaled_src_rect.size.x < (unsigned int) -dst_pos.x)
               {
                  return;
               }
               scaled_src_rect.pos.x += -dst_pos.x;
               scaled_src_rect.size.x += dst_pos.x;
            }
            if(dst_pos.y < 0)
            {
               if(scaled_src_rect.size.y < (unsigned int) -dst_pos.y)
               {
                  return;
               }
               scaled_src_rect.pos.y += -dst_pos.y;
               scaled_src_rect.size.y += dst_pos.y;
            }
            scaled_src_rect.pos *= scale_numerator;
            scaled_src_rect.pos /= scale_denominator;
            scaled_src_rect.size *= scale_numerator;
            scaled_src_rect.size /= scale_denominator;

            Point scaled_dst_pos(dst_pos);
            if(dst_pos.x < 0)
            {
               scaled_dst_pos.x = 0;
            }
            if(dst_pos.y < 0)
            {
               scaled_dst_pos.y = 0;
            }
            scaled_dst_pos *= scale_numerator;
            scaled_dst_pos /= scale_denominator;
            scaled_dst_pos += offset;

            Blitters::blit_blend(texture->get_transform(options, scale_numerator, scale_denominator), scaled_src_rect, window, scaled_dst_pos, options.blend);
         }

         void Window::fill(const Color &color, const Rect &rect)
         {
            assert(window);

            Uint32 mapped = SDL_MapRGBA(window->format, color.red, color.green, color.blue, color.alpha);

            Rect scaled_rect(rect);
            scaled_rect.pos *= scale_numerator;
            scaled_rect.pos /= scale_denominator;
            scaled_rect.size *= scale_numerator;
            scaled_rect.size /= scale_denominator;
            scaled_rect.pos += offset;

            SDL_FillRect(window, &scaled_rect, mapped);
         }

         void Window::fill_blend(const Color &color, const Rect &rect)
         {
            assert(window);

            Uint32 mapped = SDL_MapRGB(window->format, color.red, color.green, color.blue);

            Rect scaled_rect(rect);
            scaled_rect.pos *= scale_numerator;
            scaled_rect.pos /= scale_denominator;
            scaled_rect.size *= scale_numerator;
            scaled_rect.size /= scale_denominator;
            scaled_rect.pos += offset;

            if(color.alpha == 0xff)
            {
               SDL_FillRect(window, &scaled_rect, mapped);
            }
            else if(color.alpha != 0x00)
            {
               SDL_Surface *temp = SDL_CreateRGBSurface(window->flags, scaled_rect.size.x, scaled_rect.size.y, window->format->BitsPerPixel, window->format->Rmask, window->format->Gmask, window->format->Bmask, window->format->Amask);

               SDL_FillRect(temp, 0, mapped);
               SDL_SetAlpha(temp, SDL_SRCALPHA | SDL_RLEACCEL, color.alpha);
               SDL_BlitSurface(temp, 0, window, &scaled_rect);
               SDL_FreeSurface(temp);
            }
         }
      }
   }
}
