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

#include "editor/editor.hpp"
#include "editor/editor_event_handling.hpp"
#include "editor/editor_layer.hpp"
#include "editor/menu_list_item.hpp"
#include "editor/menu_list_separator.hpp"
#include "editor/widget.hpp"
#include "math/rectf.hpp"

class LayersWidget;

class LayersListItem : public MenuListItem
{
public:
  LayersListItem(EditorLayersWidget* parent, EditorLayer* layer);

  virtual void draw(DrawingContext& context) override;

public:
  SurfacePtr get_state_icon() const;

private:
  bool has_visibility_settings() const;
  void toggle_visibility();

private:
  EditorLayer *m_layer;
  Vector m_eye_offset;
  SurfacePtr m_eye_surface;
  SurfacePtr m_eye_half_surface;
};