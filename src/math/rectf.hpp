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

#include "math/anchor_point.hpp"
#include "math/sizef.hpp"
#include "math/vector.hpp"

class Rect;

/** This class represents a rectangle.
 * (Implementation Note) We're using upper left and lower right point instead of
 * upper left and width/height here, because that makes the collision detection
 * a little bit more efficient.
 */
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

public:
  Rectf() :
    m_p1(),
    m_p2()
  { }

  Rectf(const Vector& np1, const Vector& np2) :
    m_p1(np1), m_p2(np2)
  {
  }

  Rectf(float x1, float y1, float x2, float y2) :
    m_p1(x1, y1), m_p2(x2, y2)
  {
    assert(m_p1.x <= m_p2.x && m_p1.y <= m_p2.y);
  }

  Rectf(const Vector& p1, const Sizef& size) :
    m_p1(p1),
    m_p2(p1.x + size.width, p1.y + size.height)
  {
  }

  Rectf(const Rect& rect);

  float& get_left() { return m_p1.x; }
  float& get_top() { return m_p1.y; }

  float get_left() const { return m_p1.x; }
  float get_right() const { return m_p2.x; }
  float get_top() const { return m_p1.y; }
  float get_bottom() const { return m_p2.y; }

  float get_width() const { return m_p2.x - m_p1.x; }
  float get_height() const { return m_p2.y - m_p1.y; }

  void set_left(float v) { m_p1.x = v; }
  void set_right(float v) { m_p2.x = v; }
  void set_top(float v) { m_p1.y = v; }
  void set_bottom(float v) { m_p2.y = v; }

  Vector get_middle() const { return Vector((m_p1.x + m_p2.x) / 2, (m_p1.y + m_p2.y) / 2); }

  void set_pos(const Vector& v) { move(v - m_p1); }

  void set_height(float height) { m_p2.y = m_p1.y + height; }
  void set_width(float width) { m_p2.x = m_p1.x + width; }
  void set_size(float width, float height)
  {
    set_width(width);
    set_height(height);
  }
  Sizef get_size() const
  {
    return Sizef(get_width(), get_height());
  }

  void move(const Vector& v)
  {
    m_p1 += v;
    m_p2 += v;
  }

  bool contains(const Vector& v) const
  {
    return v.x >= m_p1.x && v.y >= m_p1.y && v.x < m_p2.x && v.y < m_p2.y;
  }
  bool contains(const Rectf& other) const
  {
    if (m_p1.x >= other.get_right() || other.get_left() >= m_p2.x)
      return false;
    if (m_p1.y >= other.get_bottom() || other.get_top() >= m_p2.y)
      return false;

    return true;
  }

  float distance (const Vector& other, AnchorPoint ap = ANCHOR_MIDDLE) const
  {
    Vector v = get_anchor_pos (*this, ap);
    return ((v - other).norm ());
  }

  float distance (const Rectf& other, AnchorPoint ap = ANCHOR_MIDDLE) const
  {
    Vector v1 = get_anchor_pos (*this, ap);
    Vector v2 = get_anchor_pos (other, ap);

    return ((v1 - v2).norm ());
  }

  Rectf grown(float border) const
  {
    return Rectf(m_p1.x - border, m_p1.y - border,
                 m_p2.x + border, m_p2.y + border);
  }

  // leave these two public to save the headaches of set/get functions for such
  // simple things :)

  Vector p1() const { return m_p1; }
  Vector p2() const { return m_p2; }

  void set_p1(const Vector& p) { m_p1 = p; }
  void set_p2(const Vector& p) { m_p2 = p; }

private:
  /// upper left edge
  Vector m_p1;
  /// lower right edge
  Vector m_p2;
};

std::ostream& operator<<(std::ostream& out, const Rectf& rect);

#endif

/* EOF */
