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

#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "editor/widget.hpp"

#include "editor/object_icon.hpp"
#include "editor/object_info.hpp"
#include "math/fwd.hpp"
#include "util/uid.hpp"

class DrawingContext;
class Editor;
class EditorTilebox;
class MenuListWidget;
class GameObject;
class EditorLayer;
class TileMap;
class Tip;

/** A widget at the bottom of the screen for switching between tilemap
    layers and other non-movable GameObjects */
class EditorLayersWidget final : public Widget
{
public:
  enum class HoveredItem {
    NONE, SPAWNPOINTS, SECTOR, LAYERS
  };

public:
  EditorLayersWidget(Editor& editor);

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;

  virtual bool event(const SDL_Event& ev) override;
  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent& motion) override;
  virtual bool on_mouse_wheel(const SDL_MouseWheelEvent& wheel) override;

  virtual void setup() override;
  virtual void on_window_resize() override;

  void refresh();

  void refresh_sector_text();
  void refresh_layers();
  void sort_layers();

  /**
   * Adds a new layer to the layers widget.
   * @param GameObject* layer GameObject to add to layers
   * @param sort_layers_after_adding If true, the layer list will be sorted after each addition
   */
  void add_layer(GameObject* layer, bool sort_layers_after_adding = true);

  /**
   * Populates the layers menu with all the relevant layers
   */
  void populate_layers_list_widget();

  void update_current_tip();

  bool has_mouse_focus() const;


  const std::vector<std::unique_ptr<EditorLayer>>& get_editor_layers() const
  {
    return m_editor_layers;
  }

  /**
   * Returns the layer the mouse is currently hovering over
   */
  inline const std::unique_ptr<EditorLayer>& get_hovered_layer() const
  {
    return m_editor_layers[m_hovered_layer_idx];
  }

  TileMap* get_selected_tilemap() const;
  void set_selected_tilemap(TileMap* tilemap);

private:
  Vector get_layer_coords(const int idx) const;
  int get_layer_idx(const Vector& coords) const;

  void update_tip();
  void remove_invalid_layers();

private:
  Editor& m_editor;
  std::vector<std::unique_ptr<EditorLayer>> m_editor_layers;
  UID m_selected_tilemap;

  ObjectIcon m_add_icon;
  std::unique_ptr<EditorTilebox> m_add_layer_box;
  bool m_add_layer_box_visible;

  int m_Ypos;
  const int m_Xpos = 32;
  int m_Width;
  int m_scroll;
  int m_scroll_speed;

  std::string m_sector_text;
  int m_sector_text_width;

  HoveredItem m_hovered_item;
  unsigned int m_hovered_layer_idx;

  std::unique_ptr<Tip> m_object_tip;
  std::unique_ptr<MenuListWidget> m_layer_list_widget;
  bool m_layer_list_widget_visible;

  bool m_has_mouse_focus;

private:
  EditorLayersWidget(const EditorLayersWidget&) = delete;
  EditorLayersWidget& operator=(const EditorLayersWidget&) = delete;
};
