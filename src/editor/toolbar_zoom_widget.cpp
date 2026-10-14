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

#include "editor/toolbar_zoom_widget.hpp"

#include "editor/editor_camera.hpp"
#include "editor/menu_list_item.hpp"
#include "object/camera.hpp"

EditorToolbarZoomWidget::EditorToolbarZoomWidget() :
  EditorToolbarBaseWidget(),
  m_control_textbox(std::make_unique<ControlTextboxFloat>()),
  m_scale_ptr(nullptr)
{
  setup();
}

void
EditorToolbarZoomWidget::setup()
{
  set_control_callbacks();
  set_dropdown_values();
}

void
EditorToolbarZoomWidget::set_control_callbacks()
{
  auto editor = Editor::current();
  auto editor_camera = editor->get_camera();
  auto editor_project = editor->get_project();

  m_control_textbox->set_step_size(0.1f);
  m_control_textbox->set_large_step_size(1.f);
  m_control_textbox->m_validate_float = [](ControlTextboxFloat* textbox, float value)
  {
    if (value > CAMERA_MAX_ZOOM)
    {
      textbox->set_value(CAMERA_MAX_ZOOM);
      return false;
    }

    if (value < CAMERA_MIN_ZOOM)
    {
      textbox->set_value(CAMERA_MIN_ZOOM);
      return false;
    }

    return true;
  };

  auto on_change = [editor_camera]
  {
    editor_camera->keep_in_bounds();
  };
  m_control_textbox->m_on_change_callbacks.push_back(on_change);

  auto on_sector_set = [this, editor_project]
  {
    auto sector = editor_project->get_sector();
    if (sector != nullptr)
    {
      auto& camera = sector->get_camera();
      m_scale_ptr = camera.get_scale_ptr();
      m_control_textbox->bind_value(m_scale_ptr);
    }
  };
  editor_project->set_on_sector_set_callback(on_sector_set);
}

void
EditorToolbarZoomWidget::set_dropdown_values()
{
  auto editor_camera = Editor::current()->get_camera();
  auto menu_list = get_menu_list();
  float zoom_factors[] = { 0.4f, 0.5f, 0.66f, 0.75f, 1.f, 1.25f, 1.5f, 2.f, 2.5f, 3.f };

  for(const auto& zoom_factor : zoom_factors)
  {
    auto item_label = std::to_string((int)(zoom_factor * 100)) + "%";
    auto callback = [this, editor_camera, zoom_factor] {
      set_scale_value(zoom_factor);
      editor_camera->keep_in_bounds();
    };

    auto is_selected_handler = [this, zoom_factor]
    {
      auto scale_value = get_scale_value();
      return scale_value == zoom_factor;
    };

    auto zoom_item = std::make_unique<MenuListItem>(item_label, callback);
    zoom_item->set_is_selected_handler(is_selected_handler);
    menu_list->add_item(std::move(zoom_item));
  }
}

void
EditorToolbarZoomWidget::update(float dt_sec)
{
  EditorToolbarBaseWidget::update(dt_sec);
  m_control_textbox->update(dt_sec);
}

void
EditorToolbarZoomWidget::draw(DrawingContext& context)
{
  EditorToolbarBaseWidget::draw(context);

  m_control_textbox->draw(context);
}

void
EditorToolbarZoomWidget::set_position(const Vector& position)
{
  m_rect = Rectf(position, position + Vector(64, 32));
  EditorToolbarBaseWidget::set_position(position);

  auto control_rect = Rectf(position + Vector(4, 5), position + Vector(60, 25));
  m_control_textbox->set_rect(control_rect);
}