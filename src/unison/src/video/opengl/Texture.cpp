//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "Texture.hpp"
#include <unison/video/Surface.hpp>
#include <unison/video/Window.hpp>
#include <unison/video/Renderers.hpp>
#include <unison/video/backend/Renderer.hpp>

#include <assert.h>
#include <algorithm>

namespace
{
   int next_power_of_two(int val)
   {
      int result = 1;
      while(result < val)
         result *= 2;
      return result;
   }
}

namespace Unison
{
   namespace Video
   {
      namespace OpenGL
      {
         Texture::Texture(const Surface &surface) :
            Backend::Texture(surface),
            handles()
         {
         }

         /*Texture::Texture(const Surface &surface, const std::string &name) :
            Backend::Texture(surface, name),
            handles()
         {
         }

         Texture::Texture(Backend::Texture *texture) :
            Backend::Texture(texture),
            handles()
         {
         }*/

         Texture::~Texture()
         {
            for(std::vector<Handle>::iterator iter = handles.begin(), end = handles.end();iter != end;++iter)
            {
               glDeleteTextures(1, &iter->texture);
            }
         }

         const Surface Texture::get_surface()
         {
            if(surface.get_size() == Area())
            {
               assert(!handles.empty());
               surface = Surface(size);
               for(std::vector<Handle>::iterator iter = handles.begin(), end = handles.end();iter != end;++iter)
               {
                  Surface section(iter->rect.size);
                  glBindTexture(GL_TEXTURE_2D, iter->texture);
                  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, section.get_pixels());
                  surface.blit(section, iter->rect.pos, Rect(), BLEND_NONE);
               }
               assert(!glGetError());
            }
            return surface;
         }

         void Texture::save()
         {
            get_surface();
            for(std::vector<Handle>::iterator iter = handles.begin(), end = handles.end();iter != end;++iter)
            {
               glDeleteTextures(1, &iter->texture);
            }
            handles.clear();
         }

         void Texture::blit(const Surface &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
         {
            save();
            Renderers::get().get_renderer().blit(src, src_rect, surface, dst_pos, options);
         }

         void Texture::blit(const Video::Texture &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
         {
            save();
            Texture *texture = dynamic_cast<Texture *>(Backend::Texture::get_texture(src.get_id()));
            Renderers::get().get_renderer().blit(texture, src_rect, surface, dst_pos, options);
         }

         void Texture::fill(const Color &color, const Rect &rect)
         {
            save();
            Renderers::get().get_renderer().fill(surface, color, rect);
         }

         void Texture::fill_blend(const Color &color, const Rect &rect)
         {
            save();
            Renderers::get().get_renderer().fill_blend(surface, color, rect);
         }

         void Texture::blit_draw_buffer(const Rect &src_rect, const Point &dst_pos, const RenderOptions &options)
         {
            if(handles.empty())
            {
               assert(surface.get_size() != Area());
               GLint max_size;
               glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
               for(unsigned int y = 0;y < surface.get_size().y;y += max_size)
               {
                  for(unsigned int x = 0;x < surface.get_size().x;x += max_size)
                  {
                     Rect rect;
                     rect.pos.x = x;
                     rect.pos.y = y;
                     rect.size.x = std::min(surface.get_size().x - x, (unsigned int)max_size);
                     rect.size.y = std::min(surface.get_size().y - y, (unsigned int)max_size);
                     handles.push_back(create_handle(surface, rect));
                  }
               }
               surface = Surface();
            }

            glColor4ub(options.color.red, options.color.green, options.color.blue, options.alpha);
            switch(options.blend)
            {
               case BLEND_NONE:
                  glDisable(GL_BLEND);
                  //glBlendFunc(GL_ONE, GL_ZERO);
                  break;
               case BLEND_MASK:
                  assert(0 && "Mask blending not implemented");
                  return;
               case BLEND_ALPHA:
                  glEnable(GL_BLEND);
                  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                  break;
               case BLEND_ADD:
                  glEnable(GL_BLEND);
                  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                  break;
               case BLEND_MOD:
                  glEnable(GL_BLEND);
                  glBlendFunc(GL_ZERO, GL_SRC_COLOR);
                  break;
               default:
                  assert(0 && "Unrecognized blend mode");
                  return;
            }

            glPushMatrix();

            glTranslatef(dst_pos.x - src_rect.pos.x, dst_pos.y - src_rect.pos.y, 0);

            for(std::vector<Handle>::iterator iter = handles.begin(), end = handles.end();iter != end;++iter)
            {
               Rect overlap = iter->rect.get_overlap(src_rect);
               if(overlap != Rect())
               {
                  GLfloat uv_left = GLfloat(overlap.get_left()) / iter->rect.size.x;
                  GLfloat uv_top = GLfloat(overlap.get_top()) / iter->rect.size.y;
                  GLfloat uv_right = GLfloat(overlap.get_right()) / iter->rect.size.x;
                  GLfloat uv_bottom = GLfloat(overlap.get_bottom()) / iter->rect.size.y;

                  if(options.h_flip)
                  {
                     std::swap(uv_left, uv_right);
                  }

                  if(options.v_flip)
                  {
                     std::swap(uv_top, uv_bottom);
                  }

                  glPushMatrix();

                  glTranslatef(overlap.pos.x, overlap.pos.y, 0);

                  glBindTexture(GL_TEXTURE_2D, iter->texture);
                  glBegin(GL_QUADS);
                     glTexCoord2f(uv_left, uv_top);
                     glVertex2i(0, 0);

                     glTexCoord2f(uv_right, uv_top);
                     glVertex2i(overlap.size.x, 0);

                     glTexCoord2f(uv_right, uv_bottom);
                     glVertex2i(overlap.size.x, overlap.size.y);

                     glTexCoord2f(uv_left, uv_bottom);
                     glVertex2i(0, overlap.size.y);
                  glEnd();

                  glPopMatrix();
               }
            }

            glPopMatrix();

            //glColor4ub(0xff, 0xff, 0xff, 0xff);
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         }

         Texture::Handle Texture::create_handle(const Surface &surface, const Rect &rect)
         {
            Texture::Handle handle;
            handle.rect = rect;
            handle.rect.size.x = next_power_of_two(rect.size.x);
            handle.rect.size.y = next_power_of_two(rect.size.y);
            Surface convert(handle.rect.size);
            convert.blit(surface, Point(), rect, BLEND_NONE);


            glGenTextures(1, &handle.texture);
            assert(!glGetError());
            try
            {
               glBindTexture(GL_TEXTURE_2D, handle.texture);
               //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

               //surface.lock();
               glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     handle.rect.size.x, handle.rect.size.y,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, convert.get_pixels());
               //surface.unlock();

               assert(!glGetError());

               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            }
            catch(...)
            {
               glDeleteTextures(1, &handle.texture);
               throw;
            }
            return handle;
         }
      }
   }
}
