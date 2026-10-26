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

#include "editor/button_widget.hpp"
#include "editor/menu_list_base_item.hpp"

class DrawingContext;
class EditorLayersWidget;

/**
 * Class that contains buttons for layer actions (delete, add, move up / move down)
 */
class MenuListLayerActionsItem: public MenuListBaseItem
{
public:
    MenuListLayerActionsItem(EditorLayersWidget* parent);

private:
  void initialize();
  void add_delete_button();
  void add_move_up_button();
  void add_move_down_button();
  void add_layer_add_button();

public:
  virtual void update(float /* dt_sec */) override;
  virtual void draw(DrawingContext &context) override;

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent &button) override;
  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent &button) override;
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent &motion) override;

  virtual bool is_user_removable() const override { return false; }

private:
  EditorLayersWidget *m_parent;
  std::vector<std::unique_ptr<EditorToolbarButtonWidget>> m_buttons;
  std::unique_ptr<MenuListWidget> m_add_layer_menu;
  bool m_initialized;
  bool m_add_layer_menu_visible;
};