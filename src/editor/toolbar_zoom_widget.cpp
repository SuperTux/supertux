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
#include "object/camera.hpp"

EditorToolbarZoomWidget::EditorToolbarZoomWidget() :
  EditorToolbarBaseWidget(),
  m_control_textbox(std::make_unique<ControlTextboxFloat>())
{
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

  auto editor = Editor::current();
  auto editor_project = editor->get_project();
  auto on_sector_set = [this, editor_project]
  {
    auto sector = editor_project->get_sector();
    if (sector != nullptr)
    {
      auto& camera = sector->get_camera();
      m_control_textbox->bind_value(camera.get_scale_ptr());
    }
  };
  editor_project->set_on_sector_set_callback(on_sector_set);
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
  EditorToolbarBaseWidget::set_position(position);

  auto control_rect = Rectf(position + Vector(4, 5), position + Vector(60, 25));
  m_control_textbox->set_rect(control_rect);
}