//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_EDITOR_TOOLBOX_WIDGET_HPP
#define HEADER_SUPERTUX_EDITOR_TOOLBOX_WIDGET_HPP

#include <stdexcept>

#include "control/input_manager.hpp"
#include "editor/widget.hpp"
#include "math/vector.hpp"
#include "supertux/screen.hpp"
#include "supertux/tile_set.hpp"
#include "util/log.hpp"

class Editor;
class ObjectInfo;
class Rectf;
class TileSelection;
class ToolIcon;
class Tip;

/** The toolbox is on the right side of the screen and allows
    selection of the current tool and contains the object or tile
    palette */
class EditorToolboxWidget final : public Widget
{
public:
  enum class HoveredItem {
    NONE, TILEGROUP, OBJECTS, TILE, TOOL
  };

  enum class TileScrolling {
    NONE, UP, DOWN
  };

  enum class InputType {
    NONE, TILE, OBJECT
  };

public:
  EditorToolboxWidget(Editor& editor);

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent& motion) override;
  virtual bool on_mouse_wheel(const SDL_MouseWheelEvent& wheel) override;

  virtual void setup() override;
  virtual void resize() override;

  void update_mouse_icon();

  int get_tileselect_select_mode() const;
  int get_tileselect_move_mode() const;

  void select_tilegroup(int id);
  void select_objectgroup(int id);

  const ObjectInfo& get_object_info() const { return *m_object_info; }
  InputType get_input_type() const { return m_input_type; }
  void set_input_type(InputType input_type) { m_input_type = input_type; }

  std::string get_object() const { return m_object; }
  TileSelection* get_tiles() const { return m_tiles.get(); }

  bool has_mouse_focus() const;
  bool has_active_object_tip() const { return m_object_tip != nullptr; }

  void show_tile_in_toolbox(uint32_t tile);
  void show_object_in_toolbox(const std::string& classname);

private:
  Vector get_tile_coords(const int pos) const;
  int get_tile_pos(const Vector& coords) const;
  Vector get_tool_coords(const int pos) const;
  int get_tool_pos(const Vector& coords) const;

  Rectf get_item_rect(const HoveredItem& item) const;

  void update_selection();
  Rectf normalize_selection() const;
  Rectf selection_draw_rect() const;

  void draw_tilegroup(DrawingContext&);
  void draw_objectgroup(DrawingContext&);

private:
  Editor& m_editor;

  std::unique_ptr<TileSelection> m_tiles;

  std::string m_object;
  std::unique_ptr<Tip> m_object_tip;
  InputType m_input_type;

  std::unique_ptr<Tilegroup> m_active_tilegroup;
  int m_active_objectgroup;
  std::unique_ptr<ObjectInfo> m_object_info;

  std::unique_ptr<ToolIcon> m_rubber;
  std::unique_ptr<ToolIcon> m_select_mode;
  std::unique_ptr<ToolIcon> m_move_mode;
  std::unique_ptr<ToolIcon> m_undo_mode;

  HoveredItem m_hovered_item;
  int m_hovered_tile;
  TileScrolling m_tile_scrolling;
  bool m_using_scroll_wheel;
  int m_wheel_scroll_amount;
  int m_starting_tile;
  bool m_dragging;
  Vector m_drag_start;

  int m_Xpos;
  const int m_Ypos = 96;

  Vector m_mouse_pos;
  bool m_has_mouse_focus;

private:
  EditorToolboxWidget(const EditorToolboxWidget&) = delete;
  EditorToolboxWidget& operator=(const EditorToolboxWidget&) = delete;
};

#endif

/* EOF */
