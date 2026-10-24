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
  m_delete_button(std::make_unique<EditorToolbarButtonWidget>(
    "images/engine/editor/layer-delete.png", get_rect().p1(),
    []{}, Sizef(25, 25)))
{
  auto delete_current_layer = [this]
  {
    auto selected_tilemap = m_parent->get_selected_tilemap();
    if (selected_tilemap == nullptr)
    {
      return;
    }
    log_warning << "Delete layer" << std::endl;
  };
  m_delete_button->set_onclick_handler(delete_current_layer);
  m_delete_button->set_flat(true);
  m_delete_button->set_layer(LAYER_GUI + 1);
  m_delete_button->set_help_text(_("Delete layer"));
}

void
MenuListLayerActionsItem::update(float dt_sec)
{
  m_delete_button->update(dt_sec);
  m_delete_button->set_position(get_rect().p1());
}

bool
MenuListLayerActionsItem::on_mouse_button_up(const SDL_MouseButtonEvent &button)
{
  return m_delete_button->on_mouse_button_up(button);
}

bool
MenuListLayerActionsItem::on_mouse_button_down(const SDL_MouseButtonEvent &button)
{
  return m_delete_button->on_mouse_button_down(button);
}

bool
MenuListLayerActionsItem::on_mouse_motion(const SDL_MouseMotionEvent &motion)
{
  return m_delete_button->on_mouse_motion(motion);
}

void
MenuListLayerActionsItem::draw(DrawingContext& context)
{
  context.color().draw_filled_rect(m_rect, Color::BLACK, LAYER_GUI + 1);
  
  m_delete_button->draw(context);
}