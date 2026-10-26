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
#include "math/vector.hpp"
#include "video/color.hpp"

MenuListLayerActionsItem::MenuListLayerActionsItem(EditorLayersWidget* parent) :
  MenuListBaseItem(),
  m_parent(parent),
  m_buttons()
{
  add_delete_button();
  add_move_up_button();
  add_move_down_button();

  for (const auto& button : m_buttons)
  {
    button->set_flat(true);
    button->set_layer(LAYER_GUI + 1);
  }
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
MenuListLayerActionsItem::update(float dt_sec)
{
  auto position = get_rect().p1();
  for (const auto &button : m_buttons)
  {
    button->update(dt_sec);
    button->set_position(position);

    position += Vector(25, 0);
  }
}

bool
MenuListLayerActionsItem::on_mouse_button_up(const SDL_MouseButtonEvent &button)
{
  bool handled = false;
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

  for (const auto &button : m_buttons)
  {
    button->draw(context);
  }
}