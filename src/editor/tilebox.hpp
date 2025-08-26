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

#include <functional>

#include "editor/tip.hpp"
#include "interface/control_scrollbar.hpp"
#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/tile_set.hpp"

class Editor;
class ObjectGroup;
class ObjectInfo;
class Rectf;
class TileSelection;

/**
  The tilebox allows selection of tiles or objects
  from a specified tilegroup/objectgroup.

  NOTE: Not to be confused with EditorToolboxWidget, which also includes
        tilegroup/objectgroup selection and tools.
*/
class EditorTilebox final : public Widget
{
public:
  enum class HoveredItem {
    NONE, TILE, SCROLLBAR
  };

  enum class TileScrolling {
    NONE, UP, DOWN
  };

  enum class InputType {
    NONE, TILE, OBJECT
  };

public:
  EditorTilebox(Editor& editor, const Rectf& rect);

  virtual void draw(DrawingContext& context) override;

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent& motion) override;
  virtual bool on_mouse_wheel(const SDL_MouseWheelEvent& wheel) override;

  virtual void setup() override;
  virtual void on_window_resize() override;

  void set_rect(const Rectf& rect);
  inline Rectf get_rect() const { return m_rect; }

  void on_select(const std::function<void(EditorTilebox&)>& callback);

  void select_tilegroup(int id);
  void select_last_tilegroup();
  inline void set_tilegroup(std::unique_ptr<Tilegroup> tilegroup) { m_active_tilegroup = std::move(tilegroup); }
  void select_objectgroup(int id);
  void select_last_objectgroup();
  bool select_layers_objectgroup();

  inline const ObjectInfo& get_object_info() const { return *m_object_info; }
  inline InputType get_input_type() const { return m_input_type; }
  inline void set_input_type(InputType input_type) { m_input_type = input_type; }

  inline TileSelection* get_tiles() const { return m_tiles.get(); }
  inline const std::string& get_object() const { return m_object; }
  inline void set_object(const std::string& object) { m_object = object; }

  float get_tiles_height() const;

  inline bool has_active_object_tip() const { return m_object_tip->get_visible(); }
  inline size_t get_objectgroup_id() const { return m_objectgroup_id; }
  inline size_t get_tilegroup_id() const { return m_tilegroup_id; }
  
  void change_tilegroup(int dir);
  void change_objectgroup(int dir);

private:
  Vector get_tile_coords(int pos, bool relative = true) const;
  int get_tile_pos(const Vector& coords) const;
  Rectf get_tile_rect() const;

  void update_selection();
  Rectf normalize_selection(bool rounded) const;
  Rectf selection_draw_rect() const;

  void update_hovered_tile();

  void reset_scrollbar();

  void draw_tilegroup(DrawingContext& context);
  void draw_objectgroup(DrawingContext& context);

private:
  Editor& m_editor;

  Rectf m_rect;

  std::unique_ptr<TileSelection> m_tiles;

  std::string m_object;
  std::unique_ptr<Tip> m_object_tip;
  InputType m_input_type;

  std::unique_ptr<Tilegroup> m_active_tilegroup;
  ObjectGroup* m_active_objectgroup;
  std::unique_ptr<ObjectInfo> m_object_info;
  int m_tilegroup_id, m_objectgroup_id;

  std::function<void(EditorTilebox&)> m_on_select_callback;

  std::unique_ptr<ControlScrollbar> m_scrollbar;
  float m_scroll_progress;

  HoveredItem m_hovered_item;
  int m_hovered_tile;

  bool m_dragging;
  Vector m_drag_start;

  Vector m_mouse_pos;
  
  SpritePtr m_shadow;

private:
  EditorTilebox(const EditorTilebox&) = delete;
  EditorTilebox& operator=(const EditorTilebox&) = delete;
};
