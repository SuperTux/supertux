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

#include "editor/editor_layer.hpp"

#include <limits>

#include "object/tilemap.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/game_object.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

EditorLayer::EditorLayer(LayerObject* layer_object) :
  ObjectIcon("", layer_object->get_icon_path()),
  m_layer_object(layer_object),
  m_layer_visibility(),
  m_selection(is_tilemap() ? Surface::from_file("images/engine/editor/selection.png") : nullptr)
{
}

void
EditorLayer::draw(DrawingContext& context, const Vector& pos)
{
  if (!is_valid()) return;

  ObjectIcon::draw(context, pos);

  auto z_pos = get_zpos();
  if (z_pos == std::numeric_limits<int>::min())
    return;

  context.color().draw_text(Resources::small_font, std::to_string(z_pos),
                              pos + Vector(16,16),
                              ALIGN_CENTER, LAYER_GUI, ColorScheme::Menu::default_color);

  if (is_tilemap() && get_tilemap()->is_selected_in_editor())
    context.color().draw_surface(m_selection, pos, LAYER_GUI - 1);
}

void
EditorLayer::draw(DrawingContext& context, const Vector& pos, int pixels_shown)
{
  if (!is_valid()) return;

  ObjectIcon::draw(context, pos, pixels_shown);

  auto z_pos = get_zpos();
  if (z_pos == std::numeric_limits<int>::min())
    return;

  // Don't draw the text if the icon is not 100% visible
  if (is_tilemap() && get_tilemap()->is_selected_in_editor())
    context.color().draw_surface(m_selection, pos, LAYER_GUI - 1);
}

int
EditorLayer::get_zpos() const
{
  return is_valid() ? m_layer_object->get_layer() : std::numeric_limits<int>::min();
}

bool
EditorLayer::is_valid() const
{
  return m_layer_object && m_layer_object->is_valid();
}

std::string
EditorLayer::get_display_name() const
{
  auto object = get_game_object();
  auto item_label = object->get_name();
  if (item_label.empty())
  {
    item_label = object->get_display_name();
  }

  if (get_zpos() > std::numeric_limits<int>::min())
  {
    item_label += " (" + std::to_string(get_zpos()) + ")";
  }

  return item_label;
}
