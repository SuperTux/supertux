//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#ifndef HEADER_SUPERTUX_GUI_ITEM_SLIDER_HPP
#define HEADER_SUPERTUX_GUI_ITEM_SLIDER_HPP

#include "gui/menu_item.hpp"

class ItemSlider final : public MenuItem
{
public:
  ItemSlider(const std::string& text, int min_value, int max_value, int* value, const std::string& value_append = {}, int id = -1);

  /** Draws the menu item. */
  void draw(DrawingContext&, const Vector& pos, int menu_width, bool active) override;

  /** Processes the menu action. */
  void event(const SDL_Event& ev) override;

  /** Returns the minimum width of the menu item. */
  int get_width() const override;

  bool changes_width() const override { return false; }
  bool locks_selection() const override { return m_sliding; }

private:
  int m_min_value;
  int m_max_value;
  int* m_value;
  const std::string m_value_append;

  float m_slider_x;
  bool m_sliding;

private:
  ItemSlider(const ItemSlider&) = delete;
  ItemSlider& operator=(const ItemSlider&) = delete;
};

#endif

/* EOF */
