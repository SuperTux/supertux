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
#include "editor/object_menu.hpp"
#include "gui/menu_manager.hpp"
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
  m_layer_visible_surface(Surface::from_file("images/engine/editor/shown.png")),
  m_layer_hidden_surface(Surface::from_file("images/engine/editor/hidden.png")),
  m_layer_transparent_surface(Surface::from_file("images/engine/editor/transparent.png")),
  m_layer_name_textbox(std::make_unique<ControlTextbox>()),
  m_show_layer_name_textbox(),
  m_current_layer_name(),
  m_dragging(),
  m_dragging_offset()
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

  auto right_click_layer = [this]()
  {
    MenuManager::instance().push_menu(std::make_unique<ObjectMenu>(m_layer->get_game_object()));
  };
  set_on_right_click_handler(right_click_layer);

  auto tilemap = m_layer->get_tilemap();
  auto is_selected_layer = [tilemap]
  {
    return tilemap != nullptr && tilemap->is_selected_in_editor();
  };

  if (tilemap)
  {
    set_is_selected_handler(is_selected_layer); 
  }

  set_item_selected_surface(layer->get_selection_surface());

  set_label_offset(get_label_offset() + m_eye_offset);

  set_selected_indicator_offset(get_selected_indicator_offset() + m_eye_offset);

  auto layer_name_changed = [this]()
  {
    m_show_layer_name_textbox = false;
    m_layer->get_game_object()->set_name(m_current_layer_name);
    set_label(m_layer->get_display_name());
  };
  m_layer_name_textbox->m_on_change_callbacks.emplace_back(layer_name_changed);
}

bool
LayersListItem::has_focus() const
{
  return m_show_layer_name_textbox && m_layer_name_textbox->has_focus();
}

bool
LayersListItem::has_visibility_settings() const
{
  const auto &class_name = m_layer->get_game_object()->get_class_name();

  return class_name == TileMap::class_name() ||
         class_name == Gradient::class_name() ||
         class_name == Background::class_name() ||
         class_name.find("particles-") == 0;
}

bool
LayersListItem::event(const SDL_Event &ev)
{
  if (m_show_layer_name_textbox)
  {
    return m_layer_name_textbox->event(ev);
  }
  return false;
}

bool 
LayersListItem::on_mouse_button_down(const SDL_MouseButtonEvent& button)
{
  // m_dragging = true;

  auto event_handling = Editor::current()->get_event_handling();
  m_dragging_offset = event_handling->get_mouse_pos() - get_rect().p1();
  
  if (m_show_layer_name_textbox)
  {
    return m_layer_name_textbox->on_mouse_button_down(button);
  }

  auto textbox_rect = Rectf(get_rect().p1() + Vector(get_label_offset().x, 0), get_rect().p2());
  if (textbox_rect.contains(event_handling->get_mouse_pos()) && button.clicks == 2)
  {
    m_show_layer_name_textbox = true;
    m_current_layer_name = m_layer->get_game_object()->get_name();

    m_layer_name_textbox->set_rect(textbox_rect);
    m_layer_name_textbox->bind_string(&m_current_layer_name);
    return true;
  }

  return MenuListItem::on_mouse_button_down(button);
}

bool
LayersListItem::on_mouse_motion(const SDL_MouseMotionEvent& motion)
{
  if (m_show_layer_name_textbox)
  {
    return m_layer_name_textbox->on_mouse_motion(motion);
  }

  auto mouse_pos = Editor::current()->get_event_handling()->get_mouse_pos();
  if (m_dragging)
  {
    m_rect.set_pos(Vector(m_rect.p1().x, mouse_pos.y));
  }

  return MenuListItem::on_mouse_motion(motion);
}

void 
LayersListItem::draw(DrawingContext& context)
{
  MenuListItem::draw(context);
  draw_visibility_icon(context);

  if (m_show_layer_name_textbox)
  {
    m_layer_name_textbox->draw(context);
  }
}

void
LayersListItem::draw_visibility_icon(DrawingContext& context)
{
  if (!has_visibility_settings())
    return;

  auto origin = get_rect().p1() + m_eye_offset + Vector(5, 5);
  auto target_rect = Rectf(origin, origin + Vector(15, 15));

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
    case EditorLayer::SINGLE:
    case EditorLayer::VISIBLE:
      return m_layer_visible_surface;
    
    case EditorLayer::SEMI_TRANSPARENT:
      return m_layer_transparent_surface;

    case EditorLayer::HIDDEN:
      return m_layer_hidden_surface;
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

  m_parent->restore_layer_visibilities();

  if (next_visibility == EditorLayer::SINGLE)
  {
    m_parent->save_layer_visibilities();
    for(const auto& layer : m_parent->get_editor_layers())
    {
      layer->set_layer_visibility(EditorLayer::HIDDEN);
    }
    
    if (m_layer->get_tilemap() != nullptr)
    {
      m_parent->set_selected_tilemap(m_layer->get_tilemap());
    }
  }

  m_layer->set_layer_visibility(next_visibility);
}