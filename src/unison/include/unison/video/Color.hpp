//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_COLOR_HPP
#define UNISON_VIDEO_COLOR_HPP

//#include "SDL_stdinc.h"

namespace Unison
{
   namespace Video
   {
      /// A RGBA color
      class Color
      {
         public:
            /// The red component (0x00 to 0xff)
            unsigned char red;

            /// The green component (0x00 to 0xff)
            unsigned char green;

            /// The blue component (0x00 to 0xff)
            unsigned char blue;

            /// The alpha component (0x00 to 0xff)
            unsigned char alpha;

            /// Default constructor (transparent black)
            Color() :
               red(),
               green(),
               blue(),
               alpha()
            {
            }

            /// Create a color from the given values
            /// \param[in] red The red component (0x00 to 0xff)
            /// \param[in] green The red component (0x00 to 0xff)
            /// \param[in] blue The red component (0x00 to 0xff)
            /// \param[in] alpha The red component (0x00 to 0xff)
            Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0xff) :
               red(red),
               green(green),
               blue(blue),
               alpha(alpha)
            {
            }

            /// Equality operator
            /// \param[in] rhs The color to test
            /// \return Whether the colors are equal
            bool operator == (const Color &rhs) const
            {
               return red == rhs.red && green == rhs.green && blue == rhs.blue && alpha == rhs.alpha;
            }

            /// Equality operator
            /// \param[in] rhs The color to test
            /// \return Whether the colors are not equal
            bool operator != (const Color &rhs) const
            {
               return !(*this == rhs);
            }

            /// Less than operator
            /// \param[in] rhs The color to test
            /// \return Whether the color's grayscale value is less than the tested color
            bool operator < (const Color &rhs) const
            {
               return grayscale() < rhs.grayscale();
            }

            /// Calculate the grayscale value of the color
            /// \return The grayscale value (30% red, 59% green, 11% blue)
            unsigned char grayscale() const
            {
               return (red * 30 + green * 59 + blue * 11) / 100;
            }

            /// Opaque black (red = 0x00, green = 0x00, blue = 0x00)
            static const Color BLACK;

            /// Opaque red (red = 0xff, green = 0x00, blue = 0x00)
            static const Color RED;

            /// Opaque green (red = 0x00, green = 0xff, blue = 0x00)
            static const Color GREEN;

            /// Opaque blue (red = 0x00, green = 0x00, blue = 0xff)
            static const Color BLUE;

            /// Opaque cyan (red = 0x00, green = 0xff, blue = 0xff)
            static const Color CYAN;

            /// Opaque magenta (red = 0xff, green = 0x00, blue = 0xff)
            static const Color MAGENTA;

            /// Opaque yellow (red = 0xff, green = 0xff, blue = 0x00)
            static const Color YELLOW;

            /// Opaque white (red = 0xff, green = 0xff, blue = 0xff)
            static const Color WHITE;
      };
   }
}

#endif
