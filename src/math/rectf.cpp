//  SuperTux
//  Copyright (C) 2016 Ingo Ruhnke <grumbel@gmail.com>
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

#include "math/rectf.hpp"

#include <ostream>

#include "math/rect.hpp"

Rectf::Rectf(const Rect& rect) :
  m_p1(static_cast<float>(rect.left),
     static_cast<float>(rect.top)),
  m_size(static_cast<float>(rect.get_width()),
         static_cast<float>(rect.get_height()))
{
}

void
Rectf::fit(const Sizef& target_size)
{
  const float ratio = m_size.width / m_size.height;
  const float target_ratio = target_size.width / target_size.height;

  m_size = target_size;

  if (ratio > target_ratio)
    m_size.height = target_size.width / ratio;
  else if (ratio < target_ratio)
    m_size.width = target_size.height * ratio;
}

void
Rectf::fit_centered(const Sizef& target_size)
{
  fit(target_size);

  m_p1 += Vector((target_size.width - m_size.width) / 2.f,
                 (target_size.height - m_size.height) / 2.f);
}

Rect
Rectf::to_rect() const
{
  return { static_cast<int>(m_p1.x), static_cast<int>(m_p1.y),
           static_cast<int>(get_right()), static_cast<int>(get_bottom()) };
}

std::ostream& operator<<(std::ostream& out, const Rectf& rect)
{
  out << "Rectf("
      << rect.get_left() << ", " << rect.get_top() << ", "
      << rect.get_right() << ", " << rect.get_bottom()
      << ")";
  return out;
}

/* EOF */
