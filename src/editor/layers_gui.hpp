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

#ifndef HEADER_SUPERTUX_EDITOR_LAYERS_GUI_HPP
#define HEADER_SUPERTUX_EDITOR_LAYERS_GUI_HPP

#include <stdexcept>

#include "control/input_manager.hpp"
#include "supertux/screen.hpp"

class DrawingContext;
class Editor;
class GameObject;
class LayerIcon;
class Tip;
class Vector;

class EditorLayersGui final
{
public:
  static bool less_z_pos(const std::unique_ptr<LayerIcon>& lhs, const std::unique_ptr<LayerIcon>& rhs);

public:
  enum HoveredItem {
    HI_NONE, HI_SPAWNPOINTS, HI_SECTOR, HI_LAYERS
  };

public:
  EditorLayersGui(Editor& editor);

  void draw(DrawingContext&);
  void update(float dt_sec);
  bool event(SDL_Event& ev);
  void setup();
  void resize();

  void refresh_sector_text();
  void sort_layers();

  void add_layer(GameObject* layer);

private:
  Vector get_layer_coords(const int pos) const;
  int get_layer_pos(const Vector& coords) const;
  void update_tip();

private:
  Editor& m_editor;

public:
  std::vector<std::unique_ptr<LayerIcon>> m_layers;
  GameObject* m_selected_tilemap;

private:
  int m_Ypos;
  const int m_Xpos = 32;
  int m_Width;

  std::string m_sector_text;
  int m_sector_text_width;

  HoveredItem m_hovered_item;
  unsigned int m_hovered_layer;

  std::unique_ptr<Tip> m_object_tip;

private:
  EditorLayersGui(const EditorLayersGui&) = delete;
  EditorLayersGui& operator=(const EditorLayersGui&) = delete;
};

#endif

/* EOF */
