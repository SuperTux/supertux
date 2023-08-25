//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_MATH_RECTF_HPP
#define HEADER_SUPERTUX_MATH_RECTF_HPP

#include <assert.h>
#include <iosfwd>

#include <SDL.h>

#include "math/anchor_point.hpp"
#include "math/rect.hpp"
#include "math/sizef.hpp"
#include "math/vector.hpp"
#include "util/log.hpp"

class Rectf final
{
public:
  static Rectf from_center(const Vector& center, const Sizef& size)
  {
    return Rectf(center.x - size.width / 2.0f,
                 center.y - size.height / 2.0f,
                 center.x + size.width / 2.0f,
                 center.y + size.height / 2.0f);
  }

private:
  void initialize();

public:
  Rectf() :
    m_p1(0.0f, 0.0f),
    m_size()
  { }

  Rectf(const Rectf& rhs) = default;
  Rectf& operator=(const Rectf& rhs) = default;

  Rectf(const Vector& np1, const Vector& np2) :
    m_p1(np1), m_size(np2.x - np1.x, np2.y - np1.y)
  {
    initialize();
  }

  Rectf(float x1, float y1, float x2, float y2) :
    m_p1(x1, y1), m_size(x2 - x1, y2 - y1)
  {
    initialize();
  }

  Rectf(const Vector& p1, const Sizef& size) :
    m_p1(p1),
    m_size(size)
  {
    initialize();
  }

  Rectf(const SDL_FRect& rect) :
    m_p1(rect.x, rect.y), m_size(rect.w, rect.h)
  {
    initialize();
  }

  Rectf(const Rect& rect);

  bool operator==(const Rectf& other) const
  {
    return (m_p1 == other.m_p1 &&
            m_size == other.m_size);
  }

  // This is a temporary hack to pass x/y to ReaderMapping
  float& get_left() { return m_p1.x; }
  float& get_top() { return m_p1.y; }

  float get_left() const { return m_p1.x; }
  float get_right() const { return m_p1.x + m_size.width; }
  float get_top() const { return m_p1.y; }
  float get_bottom() const { return m_p1.y + m_size.height; }

  float get_width() const { return m_size.width; }
  float get_height() const { return m_size.height; }

  void set_left(float v) { m_size.width -= v - m_p1.x; m_p1.x = v; }
  void set_right(float v) { m_size.width += v - get_right(); }

  void set_top(float v) { m_size.height -= v - m_p1.y; m_p1.y = v; }
  void set_bottom(float v) { m_size.height += v - get_bottom(); }

  Vector get_middle() const { return Vector(m_p1.x + m_size.width / 2.0f,
                                            m_p1.y + m_size.height / 2.0f); }

  void set_pos(const Vector& v) { m_p1 = v; }

  void set_width(float width)
  {
    if (width < 0.f)
    {
      log_warning << "Attempted to set width to negative value: " << width << ". Setting to 0." << std::endl;
      width = 0.f;
    }
    m_size.width = width;
  }
  void set_height(float height)
  {
    if (height < 0.f)
    {
      log_warning << "Attempted to set height to negative value: " << height << ". Setting to 0." << std::endl;
      height = 0.f;
    }
    m_size.height = height;
  }
  void set_size(float width, float height)
  {
    set_width(width);
    set_height(height);
  }
  Sizef get_size() const { return m_size; }

  bool empty() const
  {
    return get_width() <= 0 ||
           get_height() <= 0;
  }

  void move(const Vector& v) { m_p1 += v; }
  Rectf moved(const Vector& v) const { return Rectf(m_p1 + v, m_size); }

  bool contains(const Vector& v) const {
    return v.x >= m_p1.x && v.y >= m_p1.y && v.x < get_right() && v.y < get_bottom();
  }

  bool contains(const Rectf& other) const
  {
    // FIXME: This is overlaps(), not contains()!
    if (m_p1.x >= other.get_right() || other.get_left() >= get_right())
      return false;
    if (m_p1.y >= other.get_bottom() || other.get_top() >= get_bottom())
      return false;

    return true;
  }

  float distance (const Vector& other, AnchorPoint ap = ANCHOR_MIDDLE) const
  {
    Vector v = get_anchor_pos (*this, ap);
    return glm::distance(v, other);
  }

  float distance (const Rectf& other, AnchorPoint ap = ANCHOR_MIDDLE) const
  {
    Vector v1 = get_anchor_pos(*this, ap);
    Vector v2 = get_anchor_pos(other, ap);

    return glm::distance(v1, v2);
  }

  Rectf grown(float border) const
  {
    return Rectf(m_p1.x - border, m_p1.y - border,
                 get_right() + border, get_bottom() + border);
  }

  // leave these two public to save the headaches of set/get functions for such
  // simple things :)

  Vector p1() const { return m_p1; }
  Vector p2() const { return Vector(m_p1.x + m_size.width, m_p1.y + m_size.height); }

  void set_p1(const Vector& p) {
    m_size = Sizef(m_size.width + (m_p1.x - p.x),
                   m_size.height + (m_p1.y - p.y));
    m_p1 = p;
  }
  void set_p2(const Vector& p) {
    m_size = Sizef(p.x - m_p1.x,
                   p.y - m_p1.y);
  }

  Rect to_rect() const
  {
    return { static_cast<int>(m_p1.x), static_cast<int>(m_p1.y),
             static_cast<int>(m_size.width), static_cast<int>(m_size.height) };
  }

  SDL_FRect to_sdl() const
  {
    return { m_p1.x, m_p1.y, m_size.width, m_size.height };
  }

private:
  /// upper left edge
  Vector m_p1;
  Sizef m_size;
};

std::ostream& operator<<(std::ostream& out, const Rectf& rect);

#endif

/* EOF */
