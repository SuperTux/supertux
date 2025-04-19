//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef SIZE_HPP
#define SIZE_HPP

#include "fwd.hpp"
#include "vector.hpp"

template<typename T>
class Size_t final
{
public:
  using type = T;

  Size_t()
      : width(T(0))
      , height(T(0))
  {}

  Size_t(T width_, T height_)
      : width(width_)
      , height(height_)
  {}

  explicit Size_t(const Vector_t<T>& v)
      : width(v.x)
      , height(v.y)
  {}

  Size_t(const Size_t& rhs) = default;
  Size_t& operator=(const Size_t& rhs) = default;

  Size_t& operator*=(T factor)
  {
    width *= factor;
    height *= factor;
    return *this;
  }

  Size_t& operator/=(T divisor)
  {
    width /= divisor;
    height /= divisor;
    return *this;
  }

  Size_t& operator+=(const Size_t& rhs)
  {
    width += rhs.width;
    height += rhs.height;
    return *this;
  }

  Size_t& operator-=(const Size_t& rhs)
  {
    width -= rhs.width;
    height -= rhs.height;
    return *this;
  }

  inline Vector_t<T> as_vector() const { return Vector_t<T>(width, height); }

  inline bool is_valid() const { return width > 0 && height > 0; }

  template<typename U>
  operator Size_t<U>() const
  {
    return Size_t<U>(U(width), U(height));
  }

public:
  T width;
  T height;
};

template<typename T>
inline Size_t<T> operator*(const Size_t<T>& lhs, T factor)
{
  return Size_t<T>(lhs.width * factor, lhs.height * factor);
}

template<typename T>
inline Size_t<T> operator*(T factor, const Size_t<T>& rhs)
{
  return Size_t<T>(rhs.width * factor, rhs.height * factor);
}

template<typename T>
inline Size_t<T> operator/(const Size_t<T>& lhs, T divisor)
{
  return Size_t<T>(lhs.width / divisor, lhs.height / divisor);
}

template<typename T>
inline Size_t<T> operator+(const Size_t<T>& lhs, const Size_t<T>& rhs)
{
  return Size_t<T>(lhs.width + rhs.width, lhs.height + rhs.height);
}

template<typename T>
inline Size_t<T> operator-(const Size_t<T>& lhs, const Size_t<T>& rhs)
{
  return Size_t<T>(lhs.width - rhs.width, lhs.height - rhs.height);
}

template<typename T>
inline bool operator==(const Size_t<T>& lhs, const Size_t<T>& rhs)
{
  return (lhs.width == rhs.width) && (rhs.height == rhs.height);
}

template<typename T>
inline bool operator!=(const Size_t<T>& lhs, const Size_t<T>& rhs)
{
  return (lhs.width != rhs.width) || (lhs.height != rhs.height);
}

std::ostream& operator<<(std::ostream& s, const Size& size);
std::ostream& operator<<(std::ostream& s, const Sizef& size);

#endif // SIZE_HPP
