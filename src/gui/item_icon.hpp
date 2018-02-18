//  SuperTux
//  Copyright (C) 2018 Tobias Markus <tobbi@supertux.org>
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

#ifndef HEADER_SUPERTUX_GUI_ITEM_ICON_HPP
#define HEADER_SUPERTUX_GUI_ITEM_ICON_HPP

#include <list>
#include <memory>
#include <SDL.h>

#include "gui/menu_item.hpp"

class ItemIcon : public MenuItem
{
  public:
    ItemIcon(const std::string& text_, int id = -1, const SurfacePtr& icon = nullptr);

  void draw(DrawingContext&, const Vector& pos, int menu_width, bool active);

  private:
    SurfacePtr icon;

  private:
    ItemIcon(const ItemIcon&);
    ItemIcon& operator=(const ItemIcon&);
};

#endif

/* EOF */
