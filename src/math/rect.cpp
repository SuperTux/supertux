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

#include "rect.hpp"

template<>
Rect::SDL_Rect_Wrapper Rect::to_sdl() const
{
  SDL_Rect_Wrapper wrapper;
  wrapper.rect_i = {m_pos.x, m_pos.y, m_size.width, m_size.height};
  return wrapper;
}

template<>
Rectf::SDL_Rect_Wrapper Rectf::to_sdl() const
{
  SDL_Rect_Wrapper wrapper;
  wrapper.rect_f = {m_pos.x, m_pos.y, m_size.width, m_size.height};
  return wrapper;
}

std::ostream& operator<<(std::ostream& out, const Rect& rect)
{
  out << "Rect(" << rect.get_left() << ", " << rect.get_top() << ", " << rect.get_right() << ", "
      << rect.get_bottom() << ")";
  return out;
}

std::ostream& operator<<(std::ostream& out, const Rectf& rect)
{
  out << "Rectf(" << rect.get_left() << ", " << rect.get_top() << ", " << rect.get_right() << ", "
      << rect.get_bottom() << ")";
  return out;
}
