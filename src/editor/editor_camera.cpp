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

#include "editor/editor_camera.hpp"

#include "editor/editor.hpp"
#include "object/camera.hpp"

EditorCamera::EditorCamera() :
  m_scroll_speed(),
  m_new_scale()
{
}

void
EditorCamera::scroll(const Vector& velocity)
{
  auto editor_project = Editor::current()->get_project();
  if (!editor_project->is_level_loaded())
    return;

  auto sector = editor_project->get_sector();
  auto& camera = sector->get_camera();

  camera.move(velocity / camera.get_current_scale());
  keep_in_bounds();
}

void
EditorCamera::update(Camera& camera, float dt_sec)
{
  auto event_handling = Editor::current()->get_event_handling();
  auto key_zoomed_pressed = event_handling->get_key_zoomed_pressed();
  auto mouse_pos = event_handling->get_mouse_pos();

  // Ensure camera is free, which is like normal but immune to the camera boundary.
  camera.set_mode(Camera::Mode::FREE);

  // If camera scale must be changed, change it here.
  if (m_new_scale != 0.f)
  {
    // Do not clamp, as to prevent pointless calls to EditorOverlayWidget::update_pos().
    if (m_new_scale >= CAMERA_MIN_ZOOM && m_new_scale <= CAMERA_MAX_ZOOM)
    {
      const bool zooming_in = camera.get_current_scale() < m_new_scale;

      camera.set_scale(m_new_scale);

      // When zooming in, focus on the position of the mouse.
      if (zooming_in && !key_zoomed_pressed && !g_config->editor_zoom_centered)
        camera.move((mouse_pos - Vector(static_cast<float>(SCREEN_WIDTH - 128),
                                        static_cast<float>(SCREEN_HEIGHT - 32)) / 2.f) / CAMERA_ZOOM_FOCUS_PROGRESSION);

      keep_in_bounds();
    }
    event_handling->set_key_zoomed_pressed(false);
    m_new_scale = 0.f;
  }

  camera.update(dt_sec);
}

void
EditorCamera::keep_in_bounds()
{
  auto overlay_widget = Editor::current()->get_overlay_widget();
  auto editor_project = Editor::current()->get_project();
  auto sector = editor_project->get_sector();
  auto& camera = sector->get_camera();

  constexpr float offset = 80.f;
#if 0
  float controls_offset_x = m_controls.size() != 0 ? -200.f : 0.f;
  float controls_offset_y = m_controls.size() != 0 ? -32.f : 0.f;
  camera.keep_in_bounds(Rectf(-offset + controls_offset_x, -offset + controls_offset_y,
                              std::max(0.0f, m_sector->get_editor_width() + 128.f / camera.get_current_scale()) + offset,
                              std::max(0.0f, m_sector->get_editor_height() + 32.f / camera.get_current_scale()) + offset));
#endif

  camera.keep_in_bounds(Rectf(-offset,
                              -offset,
                              std::max(0.f, sector->get_editor_width()) + offset + 128.f,
                              std::max(0.f, sector->get_editor_height()) + offset));

  overlay_widget->update_pos();
}
