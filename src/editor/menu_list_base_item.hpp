//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@googlemail.com>
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

#pragma once

#include "editor/editor.hpp"
#include "editor/widget.hpp"
#include "math/rectf.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"

#include <functional>
#include <vector>

class MenuListBaseItem : public Widget
{
public:
  inline static Sizef DEFAULT_SIZE = Sizef(200.f, 25.f);

public:
  MenuListBaseItem() :
    m_rect(Rectf(0, 0, 0, 0)),
    m_size(DEFAULT_SIZE)
  {
  }

  MenuListBaseItem(const Rectf& rect) :
    m_rect(rect),
    m_size(DEFAULT_SIZE)
  {
  }

  virtual bool is_separator() const { return false; }

  virtual bool has_focus() const { return false; }

public:
  const Rectf &get_rect() const { return m_rect; }
  inline void set_rect(const Rectf &rect) { m_rect = rect; }

  Sizef get_size() const { return m_size; }
  inline void set_size(const Sizef &size) { m_size = size; }

protected:
  Rectf m_rect;
  Sizef m_size;
};
