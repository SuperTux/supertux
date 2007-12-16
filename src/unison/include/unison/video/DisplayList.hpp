//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_DISPLAY_LIST_HPP
#define UNISON_VIDEO_DISPLAY_LIST_HPP

#include <unison/video/Blittable.hpp>
#include <unison/video/RenderOptions.hpp>
#include <unison/video/Surface.hpp>
#include <unison/video/Texture.hpp>

#include <string>
#include <vector>
#include <map>

namespace Unison
{
   namespace Video
   {
      class DisplayList : public Blittable
      {
         public:
            DisplayList() :
               requests()
            {
            }

            DisplayList(const std::map<int, DisplayList> &layers) :
               requests(std::for_each(layers.begin(), layers.end(), Collator()).requests)
            {
               std::for_each(requests.begin(), requests.end(), std::mem_fun(&Unison::Video::DisplayList::Request::ref));
            }

            DisplayList(const DisplayList &rhs) :
               Blittable(),
               requests(rhs.requests)
            {
               std::for_each(requests.begin(), requests.end(), std::mem_fun(&Unison::Video::DisplayList::Request::ref));
            }

            ~DisplayList()
            {
               std::for_each(requests.begin(), requests.end(), std::mem_fun(&Unison::Video::DisplayList::Request::unref));
            }

            DisplayList &operator = (const DisplayList &rhs)
            {
               std::for_each(rhs.requests.begin(), rhs.requests.end(), std::mem_fun(&Unison::Video::DisplayList::Request::ref));
               std::for_each(requests.begin(), requests.end(), std::mem_fun(&Unison::Video::DisplayList::Request::unref));
               requests = rhs.requests;
               return *this;
            }

            /// Add a request to do a surface-to-image blit
            /// \param[in] src The source surface
            /// \param[in] dst_pos The position to blit to
            /// \param[in] src_rect The part of the source surface to blit from
            /// \param[in] options Extra blit options
            /// \param[in] layer The drawing layer to sort by
            void blit(const Surface &src, const Point &dst_pos = Point(), const Rect &src_rect = Rect(), const RenderOptions &options = RenderOptions())
            {
               add_request(new SurfaceBlitRequest(src, dst_pos, src_rect, options));
            }

            /// Add a request to do a texture-to-image blit
            /// \param[in] src The source texture
            /// \param[in] dst_pos The position to blit to
            /// \param[in] src_rect The part of the source texture to blit from
            /// \param[in] options Extra blit options
            /// \param[in] layer The drawing layer to sort by
            void blit(const Texture &src, const Point &dst_pos = Point(), const Rect &src_rect = Rect(), const RenderOptions &options = RenderOptions())
            {
               add_request(new TextureBlitRequest(src, dst_pos, src_rect, options));
            }

            /// Add a request to fill a portion of the image
            /// \param[in] color The color
            /// \param[in] rect The portion to fill
            /// \param[in] layer The drawing layer to sort by
            void fill(const Color &color, const Rect &rect = Rect())
            {
               add_request(new FillRequest(color, rect));
            }

            /// Add a request to blended fill a portion of the image
            /// \param[in] color The color
            /// \param[in] rect The portion to fill
            /// \param[in] layer The drawing layer to sort by
            void fill_blend(const Color &color, const Rect &rect = Rect())
            {
               add_request(new BlendedFillRequest(color, rect));
            }

            /// Draw requests in display list onto blittable
            /// \param[in] dst The destination blittable
            void draw(Blittable *dst) const
            {
               std::for_each(requests.begin(), requests.end(), std::bind2nd(std::mem_fun(&Request::do_request), dst));
            }

            void clear()
            {
               requests.clear();
            }

            class Request
            {
               public:
                  Request() :
                     refcount(1)
                  {
                  }

                  virtual ~Request()
                  {
                  }

                  virtual void do_request(Blittable *dst) const = 0;

                  void ref()
                  {
                     refcount++;
                  }

                  void unref()
                  {
                     assert(refcount > 0);
                     refcount--;
                     if(refcount == 0)
                     {
                        delete this;
                     }
                  }
               private:
                  int refcount;
            };

            void add_request(Request *request)
            {
               requests.push_back(request);
            }
         private:
            class Collator
            {
               public:
                  void operator () (std::pair<int, DisplayList> pair)
                  {
                     requests.insert(requests.end(), pair.second.requests.begin(), pair.second.requests.end());
                  }
                  std::vector<Request *> requests;
            };

            class SurfaceBlitRequest : public Request
            {
               public:
                  SurfaceBlitRequest(const Surface &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
                   : src(src),
                     dst_pos(dst_pos),
                     src_rect(src_rect),
                     options(options)
                  {
                  }

                  void do_request(Blittable *dst) const
                  {
                     dst->blit(src, dst_pos, src_rect, options);
                  }
               private:
                  Surface src;
                  Point dst_pos;
                  Rect src_rect;
                  RenderOptions options;
            };

            class TextureBlitRequest : public Request
            {
               public:
                  TextureBlitRequest(const Texture &src, const Point &dst_pos, const Rect &src_rect, const RenderOptions &options)
                   : src(src),
                     dst_pos(dst_pos),
                     src_rect(src_rect),
                     options(options)
                  {
                  }

                  void do_request(Blittable *dst) const
                  {
                     dst->blit(src, dst_pos, src_rect, options);
                  }
               private:
                  Texture src;
                  Point dst_pos;
                  Rect src_rect;
                  RenderOptions options;
            };

            class FillRequest : public Request
            {
               public:
                  FillRequest(const Color &color, const Rect &rect)
                   : color(color),
                     rect(rect)
                  {
                  }

                  void do_request(Blittable *dst) const
                  {
                     dst->fill(color, rect);
                  }
               private:
                  Color color;
                  Rect rect;
            };

            class BlendedFillRequest : public Request
            {
               public:
                  BlendedFillRequest(const Color &color, const Rect &rect)
                   : color(color),
                     rect(rect)
                  {
                  }

                  void do_request(Blittable *dst) const
                  {
                     dst->fill_blend(color, rect);
                  }
               private:
                  Color color;
                  Rect rect;
            };

            std::vector<Request *> requests;
      };
   }
}

#endif
