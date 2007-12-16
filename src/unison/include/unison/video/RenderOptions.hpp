//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_RENDER_OPTIONS_HPP
#define UNISON_VIDEO_RENDER_OPTIONS_HPP

#include <unison/video/Color.hpp>

namespace Unison
{
   namespace Video
   {
      /// The color blending modes
      enum BlendMode
      {
         BLEND_NONE,
         BLEND_MASK,
         BLEND_ALPHA,
         BLEND_ADD,
         BLEND_MOD
      };

      /// Extra rendering options
      class RenderOptions
      {
         public:
            /// The additional color value used (alpha is ignored)
            Color color;

            /// The additional alpha value used
            unsigned char alpha;

            /// The blend mode used
            BlendMode blend;

            /// Flip rendering horizontally
            bool h_flip;

            /// Flip rendering vertically
            bool v_flip;

            /// Default constructor
            RenderOptions() :
               color(Color::WHITE),
               alpha(0xff),
               blend(BLEND_ALPHA),
               h_flip(false),
               v_flip(false)
            {
            }

            /// Create a set of render options with the given data
            /// \param[in] color The color modulation (alpha ignored)
            RenderOptions(const Color &color) :
               color(color),
               alpha(0xff),
               blend(BLEND_ALPHA),
               h_flip(false),
               v_flip(false)
            {
            }

            /// Create a set of render options with the given data
            /// \param[in] blend The blend mode
            RenderOptions(BlendMode blend) :
               color(Color::WHITE),
               alpha(0xff),
               blend(blend),
               h_flip(false),
               v_flip(false)
            {
            }
      };
   }
}

#endif
