//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VIDEO_COORD_HPP
#define UNISON_VIDEO_COORD_HPP

namespace Unison
{
   namespace Video
   {
      /// A 2 dimensional quantity in rectangular space
      template<typename T> class Coord
      {
         public:
            /// The horizontal value
            T x;

            /// The vertical value
            T y;

            /// Default constructor (0, 0)
            Coord()
             : x(),
               y()
            {
            }

            /// Create a coordinate from the given values
            /// \param[in] x The horizontal value
            /// \param[in] y The vertical value
            Coord(T x, T y)
             : x(x),
               y(y)
            {
            }

            /// Copy constructor
            /// \param[in] rhs The source coordinate
            template<typename V> Coord(const Coord<V> &rhs)
             : x(rhs.x),
               y(rhs.y)
            {
            }

            /// Assignment operator
            /// \param[in] rhs The source color
            template<typename V> Coord<T> &operator =(const Coord<V> &rhs)
            {
               x = rhs.x;
               y = rhs.y;
               return *this;
            }

            /// Equality operator
            /// \param[in] rhs The coordinate to test
            /// \return Whether the coordinates are equal
            template<typename V> bool operator ==(const Coord<V> &rhs) const
            {
               return x == rhs.x && y == rhs.y;
            }

            /// Equality operator
            /// \param[in] rhs The coordinate to test
            /// \return Whether the coordinates are not equal
            template<typename V> bool operator !=(const Coord<V> &rhs) const
            {
               return !(*this == rhs);
            }

            /// Add two coordinates and assign to the left operand
            /// \param[in] rhs The right operand
            /// \return The resultant coordinate
            template<typename V> Coord<T> &operator +=(const Coord<V> &rhs)
            {
               x += rhs.x;
               y += rhs.y;
               return *this;
            }

            /// Subtract two coordinates and assign to the left operand
            /// \param[in] rhs The right operand
            /// \return The resultant coordinate
            template<typename V> Coord<T> &operator -=(const Coord<V> &rhs)
            {
               x -= rhs.x;
               y -= rhs.y;
               return *this;
            }

            /// Multiply two coordinates and assign to the left operand
            /// \param[in] rhs The right operand
            /// \return The resultant coordinate
            template<typename V> Coord<T> &operator *=(const Coord<V> &rhs)
            {
               x *= rhs.x;
               y *= rhs.y;
               return *this;
            }

            /// Multiply two coordinates and assign to the left operand
            /// \param[in] rhs The right operand
            /// \return The resultant coordinate
            template<typename V> Coord<T> &operator /=(const Coord<V> &rhs)
            {
               x /= rhs.x;
               y /= rhs.y;
               return *this;
            }

            /// Add a coordinate with a scalar and assign to the left operand
            /// \param[in] rhs The right operand
            /// \return The resultant coordinate
            template<typename V> Coord<T> &operator +=(const V &rhs)
            {
               x += rhs;
               y += rhs;
               return *this;
            }

            /// Subtract a coordinate with a scalar and assign to the left operand
            /// \param[in] rhs The right operand
            /// \return The resultant coordinate
            template<typename V> Coord<T> &operator -=(const V &rhs)
            {
               x -= rhs;
               y -= rhs;
               return *this;
            }

            /// Multiply a coordinate with a scalar and assign to the left operand
            /// \param[in] rhs The right operand
            /// \return The resultant coordinate
            template<typename V> Coord<T> &operator *=(const V &rhs)
            {
               x *= rhs;
               y *= rhs;
               return *this;
            }

            /// Divide a coordinate with a scalar and assign to the left operand
            /// \param[in] rhs The right operand
            /// \return The resultant coordinate
            template<typename V> Coord<T> &operator /=(const V &rhs)
            {
               x /= rhs;
               y /= rhs;
               return *this;
            }

            /*T length()
            {
               double sq = x*x + y*y;
               return T(sqrt(sq) + 0.5);
            }*/
      };

      /// Add two coordinates
      /// \param[in] lhs The left operand
      /// \param[in] rhs The right operand
      /// \return The resultant coordinate
      template<typename T, typename V> const Coord<T> operator +(const Coord<T> &lhs, const Coord<V> &rhs)
      {
         return Coord<T>(lhs) += rhs;
      }

      /// Subtract two coordinates
      /// \param[in] lhs The left operand
      /// \param[in] rhs The right operand
      /// \return The resultant coordinate
      template<typename T, typename V> const Coord<T> operator -(const Coord<T> &lhs, const Coord<V> &rhs)
      {
         return Coord<T>(lhs) -= rhs;
      }

      /// Multiply two coordinates
      /// \param[in] lhs The left operand
      /// \param[in] rhs The right operand
      /// \return The resultant coordinate
      template<typename T, typename V> const Coord<T> operator *(const Coord<T> &lhs, const Coord<V> &rhs)
      {
         return Coord<T>(lhs) *= rhs;
      }

      /// Divide two coordinates
      /// \param[in] lhs The left operand
      /// \param[in] rhs The right operand
      /// \return The resultant coordinate
      template<typename T, typename V> const Coord<T> operator /(const Coord<T> &lhs, const Coord<V> &rhs)
      {
         return Coord<T>(lhs) /= rhs;
      }

      /// Add a coordinate with a scalar
      /// \param[in] lhs The left operand
      /// \param[in] rhs The right operand
      /// \return The resultant coordinate
      template<typename T, typename V> const Coord<T> operator +(const Coord<T> &lhs, const V &rhs)
      {
         return Coord<T>(lhs) += rhs;
      }

      /// Subtract a coordinate with a scalar
      /// \param[in] lhs The left operand
      /// \param[in] rhs The right operand
      /// \return The resultant coordinate
      template<typename T, typename V> const Coord<T> operator -(const Coord<T> &lhs, const V &rhs)
      {
         return Coord<T>(lhs) -= rhs;
      }

      /// Multiply a coordinate with a scalar
      /// \param[in] lhs The left operand
      /// \param[in] rhs The right operand
      /// \return The resultant coordinate
      template<typename T, typename V> const Coord<T> operator *(const Coord<T> &lhs, const V &rhs)
      {
         return Coord<T>(lhs) *= rhs;
      }

      /// Divide a coordinate with a scalar
      /// \param[in] lhs The left operand
      /// \param[in] rhs The right operand
      /// \return The resultant coordinate
      template<typename T, typename V> const Coord<T> operator /(const Coord<T> &lhs, const V &rhs)
      {
         return Coord<T>(lhs) /= rhs;
      }

      /// Add a coordinate with a scalar
      /// \param[in] lhs The left operand
      /// \param[in] rhs The right operand
      /// \return The resultant coordinate
      template<typename V, typename T> const Coord<T> operator +(const V &lhs, const Coord<T> &rhs)
      {
         return Coord<T>(rhs) += lhs;
      }

      /// Subtract a coordinate with a scalar
      /// \param[in] lhs The left operand
      /// \param[in] rhs The right operand
      /// \return The resultant coordinate
      template<typename V, typename T> const Coord<T> operator -(const V &lhs, const Coord<T> &rhs)
      {
         return Coord<T>(rhs) -= lhs;
      }

      /// Multiply a coordinate with a scalar
      /// \param[in] lhs The left operand
      /// \param[in] rhs The right operand
      /// \return The resultant coordinate
      template<typename V, typename T> const Coord<T> operator *(const V &lhs, const Coord<T> &rhs)
      {
         return Coord<T>(rhs) *= lhs;
      }

      /// Divide a coordinate with a scalar
      /// \param[in] lhs The left operand
      /// \param[in] rhs The right operand
      /// \return The resultant coordinate
      template<typename V, typename T> const Coord<T> operator /(const V &lhs, const Coord<T> &rhs)
      {
         return Coord<T>(rhs) /= lhs;
      }

      /// A point in 2-dimensional space
      typedef Coord<int> Point;

      /// A 2-dimensional area
      typedef Coord<unsigned int> Area;
   }
}

#endif
