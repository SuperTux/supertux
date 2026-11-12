//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
//                2024 Vankata453
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

#include "editor/widget.hpp"

#include <map>
#include <memory>

#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "video/surface_ptr.hpp"

template<typename T>
class ControlEnum;

class Editor;
class EditorBrush;
class EditorTilebox;
class MenuListWidget;
class ToolIcon;

/** The toolbox is on the right side of the screen and allows
    selection of the current tool and contains the object or tile
    palette. */
class EditorToolboxWidget final : public Widget
{
public:
  enum class HoveredItem {
    NONE, TILEGROUP, OBJECTS, TOOL, TILEBOX
  };

public:
  EditorToolboxWidget(Editor& editor);

  virtual void draw(DrawingContext& context) override;

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent& motion) override;
  virtual bool on_mouse_wheel(const SDL_MouseWheelEvent& wheel) override;

  virtual void setup() override;
  virtual void on_window_resize() override;

  void select_tile_group(int id);
  void select_object_group(int id);
  void select_last_tile_group();
  void select_last_object_group();

  void switch_current_group(int dir);

  int get_tileselect_select_mode() const;
  int get_object_select_mode() const;

  void update_mouse_icon();
  void set_tileselect_move_mode(int mode);

  void set_tileselect_select_mode(int mode);

  void set_mouse_tool();
  void set_rubber_tool();

  inline EditorTilebox& get_tilebox() const { return *m_tilebox; }

  inline bool has_mouse_focus() const { return m_has_mouse_focus; }

  void load_brushes();
  void save_brushes();

private:
  /**
   * Draws the line and shadow on the intersection between the
   * OverlayWidget and the tile selector on the right side.
   * @param context The current DrawingContext instance
   */
  void draw_separator(DrawingContext &context);

  ToolIcon* get_mouse_icon() const;
  void update_last_active_group();

  void prepare_tile_group_dropdown();
  void prepare_object_group_dropdown();

private:
  Editor& m_editor;

  std::unique_ptr<EditorTilebox> m_tilebox;

  float m_pos_x;
  HoveredItem m_hovered_item;
  HoveredItem m_last_active_group;
  int m_hovered_tool;

  std::unique_ptr<ToolIcon> m_rubber;
  std::unique_ptr<ToolIcon> m_select_mode;
  std::unique_ptr<ToolIcon> m_node_marker_mode;
  std::unique_ptr<ToolIcon> m_move_mode;

  std::unique_ptr<ControlEnum<int>> m_tile_groups_dropdown;
  std::unique_ptr<ControlEnum<int>> m_object_groups_dropdown;

  std::unique_ptr<MenuListWidget> m_brushes_list_widget;
  std::vector<std::unique_ptr<EditorBrush>> m_brushes;

  bool m_has_mouse_focus;

private:
  EditorToolboxWidget(const EditorToolboxWidget&) = delete;
  EditorToolboxWidget& operator=(const EditorToolboxWidget&) = delete;
};
