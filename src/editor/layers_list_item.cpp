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
  m_layer(layer),
  MenuListItem(layer->get_display_name(), {}, layer->get_icon_path(), Rectf(0, 0, 16, 16))
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
}
