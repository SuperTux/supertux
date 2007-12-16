//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "Renderer.hpp"
#include "Texture.hpp"
#include "Window.hpp"
#include <unison/video/Surface.hpp>
#include <unison/video/Window.hpp>
#include <unison/video/sdl/Blitters.hpp>
#include <unison/video/backend/Texture.hpp>
#include <unison/vfs/sdl/Utils.hpp>

#include <assert.h>

#include "SDL.h"
#include "SDL_image.h"

namespace Unison
{
   namespace Video
   {
      namespace OpenGL
      {
         Renderer::Renderer()
         {
         }

         Renderer::~Renderer()
         {
         }

         void Renderer::init()
         {
            SDL_InitSubSystem(SDL_INIT_VIDEO);

            glLoad(0);
         }

         void Renderer::quit()
         {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
         }

         std::string Renderer::get_name()
         {
            return "opengl";
         }

         bool Renderer::is_usable()
         {
            SDL_InitSubSystem(SDL_INIT_VIDEO);

            if(SDL_GL_LoadLibrary(0))
            {
               SDL_QuitSubSystem(SDL_INIT_VIDEO);
               return false;
            }

            if(!SDL_ListModes(0, SDL_OPENGL))
            {
               SDL_QuitSubSystem(SDL_INIT_VIDEO);
               return false;
            }

            /*const SDL_VideoInfo *info = SDL_GetVideoInfo();
            if(SDL_VideoModeOK(info->current_w, info->current_h, 0, SDL_OPENGL | SDL_FULLSCREEN))
            {
               SDL_QuitSubSystem(SDL_INIT_VIDEO);
               return false;
            }*/

            SDL_QuitSubSystem(SDL_INIT_VIDEO);

            return true;
         }

         Surface Renderer::load_surface(const std::string &filename)
         {
            SDL_Surface *image = IMG_Load_RW(VFS::SDL::Utils::open_physfs_in(filename), 1);
            assert(image);
            Surface surface = Surface(Area(image->w, image->h));
            SDL_Surface *sdl_surface = SDL::Blitters::create_sdl_surface_from(surface);
            assert(sdl_surface);
            SDL::Blitters::blit_blend_none(image, Rect(), sdl_surface, Point());
            SDL_FreeSurface(sdl_surface);
            return surface;
         }

         Surface Renderer::load_surface(const std::string &filename, const Color &colorkey)
         {
            SDL_Surface *image = IMG_Load_RW(VFS::SDL::Utils::open_physfs_in(filename), 1);
            assert(image);
            Surface surface = Surface(Area(image->w, image->h));
            SDL_Surface *sdl_surface = SDL::Blitters::create_sdl_surface_from(surface);
            assert(sdl_surface);
            SDL_SetColorKey(image, SDL_SRCCOLORKEY, SDL_MapRGB(image->format, colorkey.red, colorkey.blue, colorkey.alpha));
            SDL::Blitters::blit_blend_none(image, Rect(), sdl_surface, Point());
            SDL_FreeSurface(sdl_surface);
            return surface;
         }

         void Renderer::save_surface(const Surface &surface, const std::string &filename)
         {
            SDL_Surface *sdl_surface = SDL::Blitters::create_sdl_surface_from(surface);
            assert(sdl_surface);
            SDL_SaveBMP(sdl_surface, filename.c_str());
            SDL_FreeSurface(sdl_surface);
         }

         void Renderer::blit(const Surface &src, const Rect &src_rect, Surface &dst, const Point &dst_pos, const RenderOptions &options)
         {
            Surface fragment;
            if(src_rect.pos == Point() && (src_rect.size == Area() || src_rect.size == src.get_size()))
            {
               fragment = src;
            }
            else
            {
               fragment = Surface(src_rect.size);
               Blitters::blit_blend_none(src, src_rect, fragment, Point());
            }
            if(options.h_flip)
            {
               fragment = fragment.h_flip();
            }
            if(options.v_flip)
            {
               fragment = fragment.v_flip();
            }
            SDL_Surface *src_surface = SDL::Blitters::create_sdl_surface_from(fragment.modulate(options.color).modulate(options.alpha));
            assert(src_surface);

            SDL_Surface *dst_surface = SDL::Blitters::create_sdl_surface_from(dst);
            assert(dst_surface);


            SDL::Blitters::blit_blend(src_surface, Rect(Point(), fragment.get_size()), dst_surface, dst_pos, options.blend);

            SDL_FreeSurface(dst_surface);
            SDL_FreeSurface(src_surface);
         }

         void Renderer::blit(Backend::Texture *src, const Rect &src_rect, Surface &dst, const Point &dst_pos, const RenderOptions &options)
         {
            assert(src);
            assert(dst.get_size() != Area());

            blit(src->get_surface(), src_rect, dst, dst_pos, options);
         }

         void Renderer::fill(Surface &dst, const Color &color, const Rect &rect)
         {
            SDL_Surface *dst_surface = SDL::Blitters::create_sdl_surface_from(dst);
            assert(dst_surface);

            Uint32 mapped = SDL_MapRGBA(dst_surface->format, color.red, color.green, color.blue, color.alpha);
            SDL_FillRect(dst_surface, &rect, mapped);
         }

         void Renderer::fill_blend(Surface &dst, const Color &color, const Rect &rect)
         {
            SDL_Surface *dst_surface = SDL::Blitters::create_sdl_surface_from(dst);
            assert(dst_surface);

            Uint32 mapped = SDL_MapRGBA(dst_surface->format, color.red, color.green, color.blue, color.alpha);
            if(color.alpha == 0xff)
            {
               SDL_FillRect(dst_surface, &rect, mapped);
            }
            else if(color.alpha != 0x00)
            {
               SDL_Surface *temp = SDL_CreateRGBSurface(dst_surface->flags, rect.size.x, rect.size.y, dst_surface->format->BitsPerPixel, dst_surface->format->Rmask, dst_surface->format->Gmask, dst_surface->format->Bmask, dst_surface->format->Amask);

               SDL_FillRect(temp, 0, mapped);
               SDL_SetAlpha(temp, SDL_SRCALPHA | SDL_RLEACCEL, color.alpha);
               SDL_BlitSurface(temp, 0, dst_surface, &rect);
               SDL_FreeSurface(temp);
            }
         }

         Backend::Window *Renderer::create_window(const Area &size, const Area &logical_size, bool fullscreen)
         {
            return new Window(size, logical_size, fullscreen);
         }

         Backend::Texture *Renderer::create_texture(const Surface &surface)
         {
            return new Texture(surface);
         }
      }
   }
}
