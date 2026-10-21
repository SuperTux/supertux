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
#include "object/background.hpp"
#include "object/gradient.hpp"
#include "object/tilemap.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"

LayersListItem::LayersListItem(EditorLayersWidget* parent, EditorLayer* layer) :
  MenuListItem(layer->get_display_name(), {}, layer->get_icon_path(), Rectf(0, 0, 16, 16)),
  m_parent(parent),
  m_layer(layer),
  m_eye_offset(Vector(25, 0)),
  m_eye_surface(Surface::from_file("images/engine/editor/stock-eye-12.png")),
  m_eye_half_surface(Surface::from_file("images/engine/editor/stock-eye-half-12.png"))
{
  auto select_layer = [parent, tilemap = m_layer->get_tilemap(), this]
  {
    auto editor_event_handling = Editor::current()->get_event_handling();
    auto mouse_pos = editor_event_handling->get_mouse_pos();
    auto eye_origin = get_rect().p1() + m_eye_offset;
    Rectf eye_rect = Rectf(eye_origin, eye_origin + Vector(get_size().height, get_size().height));

    if (has_visibility_settings())
    {
      if (eye_rect.contains(mouse_pos))
      {
        toggle_visibility();
        return;
      }

      // Selected layers become visible by default
      m_layer->set_layer_visibility(EditorLayer::VISIBLE);
    }

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

  set_label_offset(get_label_offset() + m_eye_offset);

  set_selected_indicator_offset(get_selected_indicator_offset() + m_eye_offset);
}

bool
LayersListItem::has_visibility_settings() const
{
  const auto &class_name = m_layer->get_game_object()->get_class_name();

  return class_name == TileMap::class_name() ||
         class_name == Gradient::class_name() ||
         class_name == Background::class_name();
}

void 
LayersListItem::draw(DrawingContext& context)
{
  MenuListItem::draw(context);
  draw_visibility_icon(context);
}

void
LayersListItem::draw_visibility_icon(DrawingContext& context)
{
  if (!has_visibility_settings())
    return;

  auto origin = get_rect().p1() + m_eye_offset + Vector(5, 5);
  auto target_rect = Rectf(origin, origin + Vector(15, 15));
  
  if (m_layer->get_layer_visibility() == EditorLayer::SINGLE)
  {
    context.color().draw_text(Resources::console_font, "S", origin + Vector(7, 2),
                              FontAlignment::ALIGN_LEFT, LAYER_GUI);
    return;
  }

  auto icon = get_state_icon();

  if (icon == nullptr)
    return;

  auto icon_size = Vector(icon->get_width(), icon->get_height());
  context.color().draw_surface_scaled(icon, target_rect, LAYER_GUI);
}

SurfacePtr
LayersListItem::get_state_icon() const
{
  switch(m_layer->get_layer_visibility())
  {
    default:
    case EditorLayer::VISIBLE:
      return m_eye_surface;
    
    case EditorLayer::SEMI_TRANSPARENT:
      return m_eye_half_surface;

    case EditorLayer::HIDDEN:
      // TODO: Better graphics, maybe
      return nullptr;

    case EditorLayer::SINGLE:
      // TODO: Better graphics
      return m_eye_surface;
  }
}

void
LayersListItem::toggle_visibility()
{
  auto current_visibility = m_layer->get_layer_visibility();
  auto next_visibility = EditorLayer::VISIBLE;
  switch (current_visibility)
  {
    case EditorLayer::VISIBLE:
      next_visibility = EditorLayer::SEMI_TRANSPARENT;
      break;

    case EditorLayer::SEMI_TRANSPARENT:
      next_visibility = EditorLayer::HIDDEN;
      break;

    case EditorLayer::HIDDEN:
      next_visibility = EditorLayer::SINGLE;
      break;

    default:
    case EditorLayer::SINGLE:
      next_visibility = EditorLayer::VISIBLE;
      break;
  }

  if (next_visibility == EditorLayer::SINGLE)
  {
    m_parent->save_layer_visibilities();
    for(const auto& layer : m_parent->get_editor_layers())
    {
      layer->set_layer_visibility(EditorLayer::HIDDEN);
    }
  }
  else
  {
    m_parent->restore_layer_visibilities();
  }

  m_layer->set_layer_visibility(next_visibility);
}