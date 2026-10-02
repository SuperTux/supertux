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

#include "editor/editor_event_handling.hpp"

#include "editor/editor.hpp"
#include "editor/editor_camera.hpp"
#include "editor/editor_history_manager.hpp"
#include "editor/editor_properties_panel.hpp"
#include "editor/layers_widget.hpp"
#include "editor/toolbar_widget.hpp"
#include "gui/menu_manager.hpp"
#include "object/camera.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/moving_object.hpp"
#include "video/compositor.hpp"

EditorEventHandling::EditorEventHandling() :
  m_mouse_pos(0.f, 0.f),
  m_ctrl_pressed(false),
  m_shift_pressed(false),
  m_alt_pressed(false),
  m_key_zoomed(false),
  m_pen_down(false)
{
}

void
EditorEventHandling::on_event(const SDL_Event& ev)
{
  auto editor = Editor::current();
  auto properties_panel = editor->get_properties_panel();

  // If properties sidebar controls are active and the mouse is hovering over the sidebar,
  // do not propagate mouse events to the editor or its widgets.
  if (properties_panel->has_mouse_focus(ev, m_mouse_pos))
    return;

  switch(ev.type)
  {
    case SDL_EVENT_MOUSE_MOTION:
      m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(ev.motion.x, ev.motion.y);
      break;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
      m_ctrl_pressed = ev.key.mod & SDL_KMOD_CTRL;
      m_shift_pressed = ev.key.mod & SDL_KMOD_SHIFT;
      m_alt_pressed = ev.key.mod & SDL_KMOD_ALT;
      break;
    case SDL_EVENT_PEN_BUTTON_DOWN:
      m_pen_down = true;
      break;
    case SDL_EVENT_PEN_BUTTON_UP:
      m_pen_down = false;
      break;
  };

  handle_generic_events(ev);
  handle_move_events(ev);
  handle_camera_events(ev);
  handle_history_manager_events(ev);
  handle_toolbox_events(ev);
  handle_toolbar_events(ev);
}

void
EditorEventHandling::handle_generic_events(const SDL_Event& ev)
{
  if (ev.type != SDL_EVENT_KEY_DOWN)
    return;

  if (ev.key.key == SDLK_F6) // F6
  {
    Compositor::s_render_lighting = !Compositor::s_render_lighting;
    return;
  }

  if (!m_ctrl_pressed)
    return;

  auto editor = Editor::current();
  auto editor_project = editor->get_project();

  switch (ev.key.key)
  {
    case SDLK_T: // Test Level (Ctrl+T)
    {
      std::optional<std::pair<std::string, Vector>> test_pos = std::nullopt;

      if (m_shift_pressed && m_alt_pressed)
      {
        test_pos = editor->get_test_position();
      }
      else if (m_shift_pressed)
      {
        auto sector_name = editor_project->get_sector()->get_name();
        auto position = editor->get_overlay_widget()->get_sector_pos();
        test_pos = std::make_pair(sector_name, position);
      }

      editor->test_level(test_pos);
    }
      break;
    case SDLK_S: // Save level (Ctrl+S)
      editor_project->save_level();
      break;
    case SDLK_H: // Toggle draggables (Ctrl+H)
    {
      auto draggables_visible = editor->get_draggables_visible();
      editor->set_draggables_visible(!draggables_visible);
    }
      break;
  }
}

void
EditorEventHandling::handle_toolbar_events(const SDL_Event& ev)
{
  if (ev.type != SDL_EVENT_KEY_DOWN || !m_ctrl_pressed)
    return;

  auto editor = Editor::current();
  auto toolbar_widget = editor->get_toolbar_widget();
  
  if (ev.key.key == SDLK_X) // Ctrl+X
  {
    toolbar_widget->toggle_tile_object_mode();
  }
}

void
EditorEventHandling::handle_move_events(const SDL_Event& ev)
{
  if(ev.type != SDL_EVENT_KEY_DOWN)
    return;
  
  auto editor = Editor::current();
  auto selected_object = editor->get_selected_object();
  
  if (selected_object == nullptr)
    return;

  auto moving_object = dynamic_cast<MovingObject *>(selected_object);

  if (moving_object == nullptr)
    return;

  const auto& object_position = moving_object->get_pos();
  auto move_increment = m_shift_pressed ? 32 : 1;
  auto move_vector = Vector();
  
  if (ev.key.key == SDLK_LEFT)
  {
    move_vector = Vector(-move_increment, 0);
  }
  if (ev.key.key == SDLK_RIGHT)
  {
    move_vector = Vector(move_increment, 0);
  }
  if (ev.key.key == SDLK_UP)
  {
    move_vector = Vector(0, -move_increment);
  }
  if (ev.key.key == SDLK_DOWN)
  {
    move_vector = Vector(0, move_increment);
  }

  auto target_position = object_position + move_vector;
  moving_object->set_pos(target_position.x, target_position.y);
}

void
EditorEventHandling::handle_camera_events(const SDL_Event& ev)
{
  auto editor = Editor::current();
  auto editor_project = editor->get_project();
  auto& camera = editor_project->get_sector()->get_camera();
  auto editor_camera = editor->get_camera();

  auto toolbox_widget = editor->get_toolbox_widget();
  auto layers_widget = editor->get_layers_widget();

  if (ev.type == SDL_EVENT_KEY_DOWN)
  {
    if (m_ctrl_pressed)
      editor_camera->set_scroll_speed(16.0f);
    else if (ev.key.mod & SDL_KMOD_RSHIFT)
      editor_camera->set_scroll_speed(96.0f);

    if (m_ctrl_pressed)
    {
      switch (ev.key.key)
      {
        case SDLK_PLUS: // Zoom in
        case SDLK_EQUALS:
        case SDLK_KP_PLUS:
          m_key_zoomed = true;
          editor_camera->set_scale(camera.get_current_scale() + CAMERA_ZOOM_SENSITIVITY);
          break;
        case SDLK_MINUS: // Zoom out
        case SDLK_KP_MINUS:
          m_key_zoomed = true;
          editor_camera->set_scale(camera.get_current_scale() - CAMERA_ZOOM_SENSITIVITY);
          break;
        case SDLK_D: // Reset zoom
          editor_camera->set_scale(1.0f);
          break;
        default:
          break;
      }
    }
  }
  else if (ev.type == SDL_EVENT_KEY_UP)
  {
    if (!m_ctrl_pressed && !(ev.key.mod & SDL_KMOD_RSHIFT))
      editor_camera->set_scroll_speed(32.0f);
  }
  else if (ev.type == SDL_EVENT_MOUSE_WHEEL)
  {
    if (toolbox_widget->has_mouse_focus() || layers_widget->has_mouse_focus())
      return;

#if SDL_VERSION_ATLEAST(3, 2, 12)
    float wheel_x = g_config->precise_scrolling ? ev.wheel.x : ev.wheel.integer_x;
    float wheel_y = g_config->precise_scrolling ? ev.wheel.y : ev.wheel.integer_y;
#else
    float wheel_x = ev.wheel.x;
    float wheel_y = ev.wheel.y;
#endif
    if (g_config->invert_wheel_x)
      wheel_x *= -1.f;

    if (g_config->invert_wheel_y)
      wheel_y *= -1.f;

    // Scroll or zoom with mouse wheel, if the mouse is not over the toolbox.
    // The toolbox does scrolling independently from the main area.
    if (m_ctrl_pressed)
      editor_camera->set_scale(camera.get_current_scale() + wheel_y * CAMERA_ZOOM_SENSITIVITY);
    else
      editor_camera->scroll( {(m_shift_pressed ? wheel_y * (g_config->editor_invert_shift_scroll ? -1.f : 1.f) : wheel_x) * 40.f,
                              (m_shift_pressed ? wheel_x : wheel_y) * -40.f });
  }
}

void
EditorEventHandling::handle_history_manager_events(const SDL_Event &ev)
{
  auto editor = Editor::current();
  auto history_manager = editor->get_history_manager();

  if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
  {
    switch (ev.button.button)
    {
      case SDL_BUTTON_X1:
        history_manager->undo();
        break;
      case SDL_BUTTON_X2:
        history_manager->redo();
        break;
    }
  }
  
  if (ev.type == SDL_EVENT_KEY_DOWN && m_ctrl_pressed)
  {
    switch (ev.key.key)
    {
      case SDLK_Z:
        history_manager->undo();
        break;
      case SDLK_Y:
        history_manager->redo();
        break;
    }
  }
}

void
EditorEventHandling::handle_toolbox_events(const SDL_Event& ev)
{
  auto editor = Editor::current();
  auto toolbox_widget = editor->get_toolbox_widget();

  if (ev.type != SDL_EVENT_KEY_DOWN || !m_ctrl_pressed)
    return;

  switch (ev.key.key)
  {
    case SDLK_PAGEUP:
      toolbox_widget->switch_current_group(-1);
      break;
    case SDLK_PAGEDOWN:
      toolbox_widget->switch_current_group(1);
      break;
  }
}

void
EditorEventHandling::reset_state()
{
  m_ctrl_pressed = false;
  m_alt_pressed = false;
  m_shift_pressed = false;

  // any mouse events from earlier (i.e. in menu, testing) dont pass through
  // the editor in those states, so as a lazy hack, let's just get the mouse
  // position.

  float x, y;
  SDL_GetMouseState(&x, &y);
  m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(x, y);
}

void
EditorEventHandling::update_keyboard(const Controller& controller)
{
  auto editor = Editor::current();
  auto properties_panel = editor->get_properties_panel();

  if (!editor->has_focus() || properties_panel->has_focus())
    return;

  if (m_ctrl_pressed || m_alt_pressed || m_shift_pressed)
    return;

  auto camera = editor->get_camera();
  auto scroll_speed = camera->get_scroll_speed();

  const bool* keys = nullptr;
  keys = SDL_GetKeyboardState(nullptr);
  assert(keys != nullptr);

  if (controller.pressed(Control::ESCAPE)) {
    MenuManager::instance().set_menu(MenuStorage::EDITOR_MENU);
    return;
  }

  if (controller.pressed(Control::DEBUG_MENU) && g_config->developer_mode)
  {
    MenuManager::instance().set_menu(MenuStorage::DEBUG_MENU);
    return;
  }

  if (editor->get_selected_object() == nullptr)
  {
    if (controller.hold(Control::LEFT) || keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
      camera->scroll({ -scroll_speed, 0.0f });
    }

    if (controller.hold(Control::RIGHT) || keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
      camera->scroll({ scroll_speed, 0.0f });
    }

    if (controller.hold(Control::UP) || keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
      camera->scroll({ 0.0f, -scroll_speed });
    }

    if (controller.hold(Control::DOWN) || keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
      camera->scroll({ 0.0f, scroll_speed });
    }
  }
}