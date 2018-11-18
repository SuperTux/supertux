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

#ifndef HEADER_SUPERTUX_EDITOR_INPUT_GUI_HPP
#define HEADER_SUPERTUX_EDITOR_INPUT_GUI_HPP

#include <stdexcept>

#include "control/input_manager.hpp"
#include "editor/widget.hpp"
#include "math/vector.hpp"
#include "supertux/screen.hpp"
#include "supertux/tile_set.hpp"

class Editor;
class ObjectInput;
class Rectf;
class TileSelection;
class ToolIcon;

class EditorInputGui final : public Widget
{
public:
  enum HoveredItem {
    HI_NONE, HI_TILEGROUP, HI_OBJECTS, HI_TILE, HI_TOOL
  };

  enum TileScrolling {
    TS_NONE, TS_UP, TS_DOWN
  };

  enum InputType {
    IP_NONE, IP_TILE, IP_OBJECT
  };

public:
  EditorInputGui(Editor& editor);

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;
  virtual bool event(const SDL_Event& ev) override;
  virtual void setup() override;
  virtual void resize() override;

  void reset_pos();
  void update_mouse_icon();

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

public:
  std::unique_ptr<TileSelection> m_tiles;
  std::string m_object;
  InputType m_input_type;

  std::unique_ptr<Tilegroup> m_active_tilegroup;
  int m_active_objectgroup;
  std::unique_ptr<ObjectInput> m_object_input;

public:
  std::unique_ptr<ToolIcon> m_rubber;
  std::unique_ptr<ToolIcon> m_select_mode;
  std::unique_ptr<ToolIcon> m_move_mode;
  std::unique_ptr<ToolIcon> m_settings_mode;

private:
  HoveredItem m_hovered_item;
  int m_hovered_tile;
  TileScrolling m_tile_scrolling;
  bool m_using_scroll_wheel;
  int m_wheel_scroll_amount;
  int m_starting_tile;
  bool m_dragging;
  Vector m_drag_start;

  int m_Xpos;
  const int m_Ypos = 60;

private:
  EditorInputGui(const EditorInputGui&) = delete;
  EditorInputGui& operator=(const EditorInputGui&) = delete;
};

#endif

/* EOF */
