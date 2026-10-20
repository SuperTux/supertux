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

#include "editor/layer_object.hpp"
#include "editor/object_icon.hpp"
#include "object/tilemap.hpp"

class LayerObject;

class EditorLayer : public ObjectIcon
{
public:
  EditorLayer(LayerObject* layer_object);
  ~EditorLayer() override {}

  virtual void draw(DrawingContext& context, const Vector& pos) override;
  virtual void draw(DrawingContext& context, const Vector& pos, int pixels_shown) override;

  int get_zpos() const;
  bool is_valid() const;

  /**
   * Returns the underlying LayerObject instance of this EditorLayer
   */
  inline LayerObject* get_game_object() const { return m_layer_object; }

  /**
   * Returns true if the underlying GameObject instance is a tilemap
   */
  inline bool is_tilemap() const { return m_layer_object->get_class_name() == TileMap::class_name(); }

  /**
   * Returns the underlying TileMap instance of this layer or nullptr
   * if the layer is not a tilemap
   */
  inline TileMap* get_tilemap()
  {
    return is_tilemap() ? static_cast<TileMap *>(get_game_object()) : nullptr;
  }

  /**
   * Returns the displayed title of this layer object (as shown in the editor layers widget)
   */
  std::string get_display_name() const;

  /**
   * Returns the selection surface (red box)
   */
  const SurfacePtr get_selection_surface() const { return m_selection; }

  /**
   * Returns a file path to the icon representing this editor layer
   */
  const std::string get_icon_path() const { return get_game_object()->get_icon_path(); }

private:
  LayerObject* m_layer_object;
  SurfacePtr m_selection;

private:
  EditorLayer(const EditorLayer&) = delete;
  EditorLayer& operator=(const EditorLayer&) = delete;
};
