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

#include "editor/layers_list_item.hpp"

#include "editor/editor.hpp"
#include "editor/editor_event_handling.hpp"
#include "editor/layers_widget.hpp"
#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"

LayersListItem::LayersListItem(EditorLayersWidget* parent, EditorLayer* layer) :
  MenuListItem(layer->get_display_name(), {}, layer->get_icon_path(), Rectf(0, 0, 16, 16)),
  m_layer(layer),
  m_eye_surface(Surface::from_file("images/engine/editor/stock-eye-12.png")),
  m_eye_half_surface(Surface::from_file("images/engine/editor/stock-eye-half-12.png"))
{
  auto select_layer = [parent, tilemap = m_layer->get_tilemap()]
  {
    if (tilemap == nullptr)
      return;

    parent->set_selected_tilemap(tilemap);
  };

  set_onclick_handler(select_layer);

  auto is_selected_layer = [tilemap = m_layer->get_tilemap()]
  {
    return tilemap != nullptr && tilemap->is_selected_in_editor();
  };

  set_is_selected_handler(is_selected_layer);

  set_item_selected_surface(layer->get_selection_surface());

  set_label_offset(get_label_offset() + Vector(25, 0));

  set_selected_indicator_offset(get_selected_indicator_offset() + Vector(25, 0));
}

void 
LayersListItem::draw(DrawingContext& context)
{
  MenuListItem::draw(context);

  auto icon = get_state_icon();
  auto icon_size = Vector(icon->get_width(), icon->get_height());
  auto origin = get_rect().p1() + Vector(25, 0) + Vector(5, 5);
  auto target_rect = Rectf(origin, origin + Vector(15, 15));
  context.color().draw_surface_scaled(icon, target_rect, LAYER_GUI);
}

SurfacePtr
LayersListItem::get_state_icon() const
{
  return m_eye_surface;
}