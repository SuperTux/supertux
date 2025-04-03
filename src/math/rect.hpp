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

#ifndef RECT_HPP
#define RECT_HPP

#include <SDL.h>
#include <algorithm>
#include <assert.h>
#include <ostream>
#include <tuple>
#include <variant>

#include "anchor_point.hpp"
#include "fwd.hpp"
#include "size.hpp"
#include "vector.hpp"

template<typename T>
class Rect_t final
{
public:
  using type = T;

  union SDL_Rect_Wrapper {
    SDL_Rect rect_i;
    SDL_FRect rect_f;

    operator SDL_Rect() const { return rect_i; }
    operator SDL_FRect() const { return rect_f; }
  };

public:
  static Rect_t<T> from_center(const Vector_t<T>& center, const Size_t<T>& size)
  {
    return Rect_t<T>(center.x - size.width / T(2),
                     center.y - size.height / T(2),
                     center.x + size.width / T(2),
                     center.y + size.height / T(2));
  }

  static Rect_t<T> from_center(T center_x, T center_y, T width, T height)
  {
    return Rect_t<T>(center_x - width / T(2),
                     center_y - height / T(2),
                     center_x + width / T(2),
                     center_y + height / T(2));
  }

public:
  Rect_t()
      : m_pos(0, 0)
      , m_size()
  {}

  Rect_t(const Rect_t<T>& rhs) = default;
  Rect_t& operator=(const Rect_t<T>& rhs) = default;

  Rect_t(const Vector_t<T>& np1, const Vector_t<T>& np2)
      : m_pos(np1)
      , m_size(np2.x - np1.x, np2.y - np1.y)
  {
    assert(m_size.width >= 0 && m_size.height >= 0);
  }

  template<typename U>
  Rect_t(const Rect_t<U>& other)
      : m_pos(U(other.get_left()), U(other.get_top()))
      , m_size(U(other.get_width()), U(other.get_height()))
  {}

  Rect_t(T left, T top, T right, T bottom)
      : m_pos(left, top)
      , m_size(right - left, bottom - top)
  {
    assert(m_size.width >= 0 && m_size.height >= 0);
  }

  Rect_t(const SDL_FRect& rect)
      : m_pos(rect.x, rect.y)
      , m_size(rect.w, rect.h)
  {}

  Rect_t(const SDL_Rect& rect)
      : m_pos(rect.x, rect.y)
      , m_size(rect.w, rect.h)
  {}

  Rect_t(const Vector_t<T>& p1, const Size_t<T>& size)
      : m_pos(p1)
      , m_size(size)
  {
    assert(m_size.width >= 0 && m_size.height >= 0);
  }

  Rect_t(T x, T y, const Size_t<T>& size)
      : m_pos(x, y)
      , m_size(size)
  {
    assert(m_size.width >= 0 && m_size.height >= 0);
  }

  bool operator==(const Rect_t& other) const
  {
    return (m_pos == other.m_pos && m_size == other.m_size);
  }

  inline T& get_left() { return m_pos.x; }
  inline T& get_top() { return m_pos.y; }

  inline T get_left() const { return m_pos.x; }
  inline T get_right() const { return m_pos.x + m_size.width; }
  inline T get_top() const { return m_pos.y; }
  inline T get_bottom() const { return m_pos.y + m_size.height; }

  inline T& get_width() { return m_size.width; }
  inline T& get_height() { return m_size.height; }

  inline T get_width() const { return m_size.width; }
  inline T get_height() const { return m_size.height; }

  inline void set_left(T v)
  {
    m_size.width -= v - m_pos.x;
    m_pos.x = v;
  }
  inline void set_right(T v) { m_size.width += v - get_right(); }

  inline void set_top(T v)
  {
    m_size.height -= v - m_pos.y;
    m_pos.y = v;
  }
  inline void set_bottom(T v) { m_size.height += v - get_bottom(); }

  Vector_t<T> get_middle() const
  {
    return Vector_t<T>(m_pos.x + m_size.width / T(2), m_pos.y + m_size.height / T(2));
  }

  inline void set_pos(const Vector_t<T>& v) { m_pos = v; }

  inline void set_width(T width) { m_size.width = width; }
  inline void set_height(T height) { m_size.height = height; }

  inline void set_size(T width, T height) { m_size = Size_t<T>(width, height); }
  inline Size_t<T> get_size() const { return m_size; }

  inline T get_area() const { return get_width() * get_height(); }

  inline bool empty() const { return get_width() <= 0 || get_height() <= 0; }
  inline bool valid() const { return get_left() <= get_right() && get_top() <= get_bottom(); }

  Rect_t<T> normalized() const
  {
    return Rect_t<T>(std::min(get_left(), get_right()),
                     std::min(get_top(), get_bottom()),
                     std::max(get_left(), get_right()),
                     std::max(get_top(), get_bottom()));
  }

  inline void move(const Vector_t<T>& v) { m_pos += v; }
  inline Rect_t<T> moved(const Vector_t<T>& v) const { return Rect_t<T>(m_pos + v, m_size); }

  inline bool contains(const Vector_t<T>& v) const
  {
    return v.x >= m_pos.x && v.y >= m_pos.y && v.x < get_right() && v.y < get_bottom();
  }

  inline bool contains(const Rect_t<T>& other) const
  {
    return (get_left() <= other.get_left() && other.get_right() <= get_right()
            && get_top() <= other.get_top() && other.get_bottom() <= get_bottom());
  }

  bool overlaps(const Rect_t<T>& other) const
  {
    if (get_right() < other.get_left() || get_left() > other.get_right())
      return false;
    if (get_bottom() < other.get_top() || get_top() > other.get_bottom())
      return false;

    return true;
  }

  T distance(const Vector_t<T>& other, AnchorPoint ap = ANCHOR_MIDDLE) const
  {
    Vector_t<T> v = get_anchor_pos(*this, ap);
    return glm::distance(v, other);
  }

  T distance(const Rect_t<T>& other, AnchorPoint ap = ANCHOR_MIDDLE) const
  {
    Vector_t<T> v1 = get_anchor_pos(*this, ap);
    Vector_t<T> v2 = get_anchor_pos(other, ap);

    return glm::distance(v1, v2);
  }

  Rect_t<T> grown(T border) const
  {
    // If the size would be shrunk below 0, do not resize.
    if (m_size.width + border * 2 < 0.f || m_size.height + border * 2 < 0.f)
      return *this;

    return Rect_t(m_pos.x - border, m_pos.y - border, get_right() + border, get_bottom() + border);
  }

  // leave these two public to save the headaches of set/get functions for such
  // simple things (:

  inline const Vector_t<T>& p1() const { return m_pos; }
  inline Vector_t<T> p2() const
  {
    return Vector_t<T>(m_pos.x + m_size.width, m_pos.y + m_size.height);
  }

  void set_p1(const Vector_t<T>& p)
  {
    m_size = Size_t<T>(m_size.width + (m_pos.x - p.x), m_size.height + (m_pos.y - p.y));
    m_pos = p;
  }

  inline void set_p2(const Vector_t<T>& p) { m_size = Size_t<T>(p.x - m_pos.x, p.y - m_pos.y); }

  inline Rect to_rect() const
  {
    return {static_cast<Rect::type>(m_pos.x),
            static_cast<Rect::type>(m_pos.y),
            static_cast<Rect::type>(get_right()),
            static_cast<Rect::type>(get_bottom())};
  }

  inline Rectf to_rectf() const
  {
    return {static_cast<Rectf::type>(m_pos.x),
            static_cast<Rectf::type>(m_pos.y),
            static_cast<Rectf::type>(get_right()),
            static_cast<Rectf::type>(get_bottom())};
  }

  SDL_Rect_Wrapper to_sdl() const;

  bool operator<(const Rect_t<T>& other) const
  {
    T rl = get_left();
    T rt = get_top();
    T rr = get_right();
    T rb = get_bottom();

    T ol = other.get_left();
    T ot = other.get_top();
    T or_ = other.get_right();
    T ob = other.get_bottom();

    return std::tie(rl, rt, rr, rb) < std::tie(ol, ot, or_, ob);
  }

private:
  Vector_t<T> m_pos;
  Size_t<T> m_size;
};

std::ostream& operator<<(std::ostream& out, const Rect& rect);
std::ostream& operator<<(std::ostream& out, const Rectf& rect);

#endif // RECT_HPP
