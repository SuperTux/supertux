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

#include "editor/menu_list_layer_actions_item.hpp"

#include "editor/layers_widget.hpp"
#include "editor/menu_list_item.hpp"
#include "math/vector.hpp"
#include "video/color.hpp"

MenuListLayerActionsItem::MenuListLayerActionsItem(EditorLayersWidget* parent) :
  MenuListBaseItem(),
  m_parent(parent),
  m_buttons(),
  m_initialized(),
  m_add_layer_menu(std::make_unique<MenuListWidget>())
{
}

void
MenuListLayerActionsItem::add_delete_button()
{
  auto delete_button = std::make_unique<EditorToolbarButtonWidget>(
      "images/engine/editor/layer-delete.png", get_rect().p1(),
      [] {}, Sizef(25, 25));
  auto delete_current_layer = [this]
  {
    m_parent->delete_selected_layer();
  };
  delete_button->set_onclick_handler(delete_current_layer);
  delete_button->set_help_text(_("Delete layer"));

  m_buttons.push_back(std::move(delete_button));
}

void
MenuListLayerActionsItem::add_move_up_button()
{
  auto move_up_button = std::make_unique<EditorToolbarButtonWidget>(
      "images/engine/editor/layer-up.png", get_rect().p1() + Vector(25, 0),
      [] {}, Sizef(25, 25));
  auto move_layer_up = [this]
  {
    m_parent->move_selected_layer_up();
  };
  move_up_button->set_onclick_handler(move_layer_up);
  m_buttons.push_back(std::move(move_up_button));
}

void
MenuListLayerActionsItem::add_move_down_button()
{
  auto move_down_button = std::make_unique<EditorToolbarButtonWidget>(
      "images/engine/editor/layer-down.png", get_rect().p1() + Vector(50, 0),
      [] {}, Sizef(25, 25));

  auto move_layer_down = [this]
  {
    m_parent->move_selected_layer_down();
  };
  move_down_button->set_onclick_handler(move_layer_down);
  m_buttons.push_back(std::move(move_down_button));
}

void
MenuListLayerActionsItem::add_layer_add_button()
{
  auto add_button = std::make_unique<EditorToolbarButtonWidget>(
      "images/engine/editor/add.png", get_rect().p2() - Vector(25, 25),
      [] {}, Sizef(25, 25));

  add_button->set_onclick_handler([this] { m_add_layer_menu_visible = true; });

  auto layer_objects = m_parent->get_tilebox()->get_layers_object_group();

  for (const auto &layer_type : layer_objects->get_icons())
  {
    auto surface_path = layer_type.get_surface()->get_filename();
    auto label = layer_type.get_object_class_name();
    auto surface_rect = Rectf(get_rect().p1(), Vector(25, 25));

    auto item = std::make_unique<MenuListItem>(label, [] {}, surface_path, surface_rect);

    m_add_layer_menu->add_item(std::move(item));
  }

  auto menu_list_widget = m_parent->get_menu_list_widget();
  auto menu_position = menu_list_widget->get_area().p2();

  m_add_layer_menu->recalculate_item_position_and_size(menu_position, true);

  m_buttons.push_back(std::move(add_button));
}

void
MenuListLayerActionsItem::initialize()
{
  add_delete_button();
  add_move_up_button();
  add_move_down_button();
  add_layer_add_button();

  for (const auto& button : m_buttons)
  {
    button->set_flat(true);
    button->set_layer(LAYER_GUI + 1);
  }
}

void
MenuListLayerActionsItem::update(float dt_sec)
{
  if (!m_initialized)
  {
    initialize();
    m_initialized = true;
  }

  if (m_add_layer_menu_visible)
  {
    m_add_layer_menu->update(dt_sec);
  }

  for (const auto &button : m_buttons)
  {
    button->update(dt_sec);
  }
}

bool
MenuListLayerActionsItem::on_mouse_button_up(const SDL_MouseButtonEvent &button)
{
  bool handled = false;

  if (m_add_layer_menu_visible)
  {
    handled |= m_add_layer_menu->on_mouse_button_up(button);
  }

  for (const auto &action_button : m_buttons)
  {
    handled |= action_button->on_mouse_button_up(button);
  }
  return handled;
}

bool
MenuListLayerActionsItem::on_mouse_button_down(const SDL_MouseButtonEvent &button)
{
  bool handled = false;
  
  if (m_add_layer_menu_visible)
  {
    handled |= m_add_layer_menu->on_mouse_button_down(button);
  }

  for (const auto &action_button : m_buttons)
  {
    handled |= action_button->on_mouse_button_down(button);
  }
  return handled;
}

bool
MenuListLayerActionsItem::on_mouse_motion(const SDL_MouseMotionEvent &motion)
{
  bool handled = false;

  if (m_add_layer_menu_visible)
  {
    handled |= m_add_layer_menu->on_mouse_motion(motion);
  }

  for (const auto &button : m_buttons)
  {
    handled |= button->on_mouse_motion(motion);
  }
  return handled;
}

void
MenuListLayerActionsItem::draw(DrawingContext& context)
{
  context.color().draw_filled_rect(m_rect, Color::BLACK, LAYER_GUI + 1);

  if (m_add_layer_menu_visible)
  {
    m_add_layer_menu->draw(context);
  }

  for (const auto &button : m_buttons)
  {
    button->draw(context);
  }
}