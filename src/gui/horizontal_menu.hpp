//  SuperTux
//  Copyright (C) 2022 Vankata453
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

#ifndef HEADER_SUPERTUX_GUI_HORIZONTAL_MENU_HPP
#define HEADER_SUPERTUX_GUI_HORIZONTAL_MENU_HPP

#include <vector>

#include "video/drawing_context.hpp"
#include "video/surface_ptr.hpp"

class HorizontalMenu
{
protected:
  static float s_max_width;
  static const float s_height;

  static const float s_width_offset;
  static const float s_item_spacing;

  static const float s_icon_y;
  static const float s_text_y;

protected:
  struct Item
  {
    const int id;
    const std::string text;
    const std::string description;
    const SurfacePtr icon;
  };

protected:
  std::vector<Item> m_items;

  Rectf m_rect;

  int m_selected_item;
  int m_item_range_begin;
  int m_item_range_end;
  float m_width;

public:
  HorizontalMenu();

  void add_item(std::string text, std::string description,
                std::string icon_file, int id = -1);

  virtual void draw(DrawingContext& context);
  virtual void draw_item(DrawingContext& context, const int& index,
                         const float& pos_x, const float& text_width);
  virtual void event(const SDL_Event& ev);

  virtual void menu_action(const Item& item) {}

  // Navigation functions
  void go_left();
  void go_right();

protected:
  void calculate_width(int begin = 0);

  // Overridable properties
  virtual bool interactable() const { return true; } // The user can interact.
  virtual float get_y() const { return 0.0f; } // Provide the Y position of the menu.

private:
  HorizontalMenu(const HorizontalMenu&) = delete;
  HorizontalMenu& operator=(const HorizontalMenu&) = delete;
};

#endif

/* EOF */
