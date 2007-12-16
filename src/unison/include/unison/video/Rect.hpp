//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_RECT_HPP
#define UNISON_VIDEO_RECT_HPP

#include <algorithm>

#include <unison/video/Coord.hpp>

#include "SDL.h"

namespace Unison
{
   namespace Video
   {
      /// Represents a rectangular area
      class Rect
      {
         public:
            /// The position of the rectangle
            Point pos;

            /// The size of the rectangle
            Area size;

            /// Default constructor
            Rect() :
               pos(),
               size(),
               rect()
            {
            }

            /// Create a rectangle from the given coordinates
            /// \param[in] pos The position of the rectangle
            /// \param[in] size The size of the rectangle
            Rect(const Point &pos, const Area &size) :
               pos(pos),
               size(size),
               rect()
            {
            }

            /// Create a rectangle from the given values
            /// \param[in] x The x-position of the rectangle
            /// \param[in] y The y-position of the rectangle
            /// \param[in] w The width of the rectangle
            /// \param[in] h The height of the rectangle
            Rect(int x, int y, int w, int h) :
               pos(x, y),
               size(w, h),
               rect()
            {
            }

            /// Equality operator
            /// \param[in] rhs The rectangle to test
            /// \return Whether the rectangles are equal
            bool operator == (const Rect &rhs) const
            {
               return pos == rhs.pos && size == rhs.size;
            }

            /// Equality operator
            /// \param[in] rhs The rectangle to test
            /// \return Whether the rectangles are not equal
            bool operator != (const Rect &rhs) const
            {
               return !(*this == rhs);
            }

            /// Get the left edge of the rectnagle
            /// \return The location of the left edge
            int get_left() const
            {
               return pos.x;
            }

            /// Get the top edge of the rectnagle
            /// \return The location of the top edge
            int get_top() const
            {
               return pos.y;
            }

            /// Get the right edge of the rectnagle
            /// \return The location of the right edge
            int get_right() const
            {
               return pos.x + size.x;
            }

            /// Get the bottom edge of the rectnagle
            /// \return The location of the bottom edge
            int get_bottom() const
            {
               return pos.y + size.y;
            }

            /// Calculate the overlap between the rectangles
            /// \param[in] rhs The rectangle to check
            /// \return The part of the rectangle that is overlapping
            Rect get_overlap(const Rect &rhs)
            {
               if(*this == Rect())
               {
                  return rhs;
               }
               if(rhs == Rect())
               {
                  return *this;
               }
               Rect overlap;
               if(get_left() < rhs.get_right())
               {
                  overlap.pos.x = std::max(get_left(), rhs.get_left());
               }
               else
               {
                  return Rect();
               }
               if(rhs.get_left() < get_right())
               {
                  overlap.size.x = std::min(rhs.get_right(), get_right()) - overlap.pos.x;
               }
               else
               {
                  return Rect();
               }
               if(get_top() < rhs.get_bottom())
               {
                  overlap.pos.y = std::max(get_top(), rhs.get_top());
               }
               else
               {
                  return Rect();
               }
               if(rhs.get_top() < get_bottom())
               {
                  overlap.size.y = std::min(rhs.get_bottom(), get_bottom()) - overlap.pos.y;
               }
               else
               {
                  return Rect();
               }
               return overlap;
            }

            /// Allow rectangles to be treated like SDL_Rect
            /// \return The equavalent SDL_Rect
            operator SDL_Rect () const
            {
               rect.x = pos.x;
               rect.y = pos.y;
               rect.w = size.x;
               rect.h = size.y;
               return rect;
            }

            /// Allow rectangles to be treated like SDL_Rect
            /// \return The internal SDL_Rect
            SDL_Rect *operator &() const
            {
               if(*this == Rect())
               {
                  return 0;
               }
               else
               {
                  rect.x = pos.x;
                  rect.y = pos.y;
                  rect.w = size.x;
                  rect.h = size.y;
                  return &rect;
               }
            }
         private:
            mutable SDL_Rect rect;
      };
   }
}

#endif
