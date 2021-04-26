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

#ifndef HEADER_SUPERTUX_EDITOR_LAYERS_WIDGET_HPP
#define HEADER_SUPERTUX_EDITOR_LAYERS_WIDGET_HPP

#include <stdexcept>

#include "control/input_manager.hpp"
#include "editor/widget.hpp"
#include "math/fwd.hpp"
#include "object/tilemap.hpp"
#include "supertux/screen.hpp"

class DrawingContext;
class Editor;
class GameObject;
class LayerIcon;
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

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_button_down(const SDL_MouseButtonEvent& button) override;
  virtual bool on_mouse_motion(const SDL_MouseMotionEvent& motion) override;
  virtual bool on_mouse_wheel(const SDL_MouseWheelEvent& wheel) override;

  virtual void setup() override;
  virtual void resize() override;

  void refresh();

  void refresh_sector_text();
  void sort_layers();
  void add_layer(GameObject* layer);

  bool has_mouse_focus() const;

  TileMap* get_selected_tilemap() const { return m_selected_tilemap; }

private:
  Vector get_layer_coords(const int pos) const;
  int get_layer_pos(const Vector& coords) const;
  void update_tip();

private:
  Editor& m_editor;
  std::vector<std::unique_ptr<LayerIcon>> m_layer_icons;
  TileMap* m_selected_tilemap;

  int m_Ypos;
  const int m_Xpos = 32;
  int m_Width;
  int m_scroll;
  int m_scroll_speed;

  std::string m_sector_text;
  int m_sector_text_width;

  HoveredItem m_hovered_item;
  unsigned int m_hovered_layer;

  std::unique_ptr<Tip> m_object_tip;

  bool m_has_mouse_focus;

private:
  EditorLayersWidget(const EditorLayersWidget&) = delete;
  EditorLayersWidget& operator=(const EditorLayersWidget&) = delete;
};

#endif

/* EOF */
