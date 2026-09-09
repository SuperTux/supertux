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
#include "gui/menu_manager.hpp"
#include "object/camera.hpp"
#include "supertux/menu/menu_storage.hpp"
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
  auto editor_project = editor->get_project();
  auto toolbar_widget = editor->get_toolbar_widget();
  auto toolbox_widget = editor->get_toolbox_widget();
  auto layers_widget = editor->get_layers_widget();
  auto& camera = editor_project->get_sector()->get_camera();

  // If properties sidebar controls are active and the mouse is hovering over the sidebar,
  // do not propagate mouse events to the editor or its widgets.
  if ((ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
       ev.type == SDL_EVENT_MOUSE_BUTTON_UP ||
       ev.type == SDL_EVENT_MOUSE_WHEEL) &&
      editor->pos_in_properties_panel(m_mouse_pos))
  {
    return;
  }

  if (ev.type == SDL_EVENT_MOUSE_MOTION)
  {
    m_mouse_pos = VideoSystem::current()->get_viewport().to_logical(ev.motion.x, ev.motion.y);

    // If properties sidebar controls are active and the mouse is hovering over the sidebar,
    // do not propagate mouse motion to the editor or its widgets.
    if (editor->pos_in_properties_panel(m_mouse_pos))
      return;
  }
  else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
  {
    switch (ev.button.button)
    {
      case SDL_BUTTON_X1:
        editor->undo();
        break;
      case SDL_BUTTON_X2:
        editor->redo();
        break;
    }
  } else {
    if (ev.type == SDL_EVENT_KEY_DOWN)
    {
      m_ctrl_pressed = ev.key.mod & SDL_KMOD_CTRL;
      m_shift_pressed = ev.key.mod & SDL_KMOD_SHIFT;
      m_alt_pressed = ev.key.mod & SDL_KMOD_ALT;

      if (m_ctrl_pressed)
        editor->set_scroll_speed(16.0f);
      else if (ev.key.mod & SDL_KMOD_RSHIFT)
        editor->set_scroll_speed(96.0f);

      if (ev.key.key == SDLK_F6)
      {
        Compositor::s_render_lighting = !Compositor::s_render_lighting;
        return;
      }
      else if (m_ctrl_pressed)
      {
        switch (ev.key.key)
        {
          case SDLK_T:
          {
            if (m_shift_pressed && m_alt_pressed)
            {
              auto test_pos = editor->get_test_position();
              editor->test_level(test_pos);
              break;
            }

            std::optional<std::pair<std::string, Vector>> test_pos = std::nullopt;
            if (m_shift_pressed)
            {
              auto sector_name = editor_project->get_sector()->get_name();
              auto position = editor->get_overlay_widget()->get_sector_pos();
              test_pos = std::make_pair(sector_name, position);
            }

            editor->test_level(test_pos);
          }
            break;
          case SDLK_S:
            editor_project->save_level();
            break;
          case SDLK_Z:
            editor->undo();
            break;
          case SDLK_Y:
            editor->redo();
            break;
          case SDLK_H:
          {
            auto show_draggables = editor->get_show_draggables();
            editor->set_show_draggables(!show_draggables);
          }
            break;
          case SDLK_X:
            toolbar_widget->toggle_tile_object_mode();
            break;
          case SDLK_PAGEUP:
            toolbox_widget->switch_current_group(-1);
            break;
          case SDLK_PAGEDOWN:
            toolbox_widget->switch_current_group(1);
            break;
          case SDLK_PLUS: // Zoom in
          case SDLK_EQUALS:
          case SDLK_KP_PLUS:
            m_key_zoomed = true;
            editor->set_camera_scale(camera.get_current_scale() + CAMERA_ZOOM_SENSITIVITY);
            break;
          case SDLK_MINUS: // Zoom out
          case SDLK_KP_MINUS:
            m_key_zoomed = true;
            editor->set_camera_scale(camera.get_current_scale() - CAMERA_ZOOM_SENSITIVITY);
            break;
          case SDLK_D: // Reset zoom
            editor->set_camera_scale(1.0f);
            break;
          default:
            break;
        }
      }
    }
    else if (ev.type == SDL_EVENT_KEY_UP)
    {
      m_ctrl_pressed = ev.key.mod & SDL_KMOD_CTRL;
      m_shift_pressed = ev.key.mod & SDL_KMOD_SHIFT;
      m_alt_pressed = ev.key.mod & SDL_KMOD_ALT;

      if (!m_ctrl_pressed && !(ev.key.mod & SDL_KMOD_RSHIFT))
        editor->set_scroll_speed(32.0f);
    }
    else if (ev.type == SDL_EVENT_PEN_BUTTON_DOWN)
    {
      m_pen_down = true;
    }
    else if (ev.type == SDL_EVENT_PEN_BUTTON_UP)
    {
      m_pen_down = false;
    }
    else if (ev.type == SDL_EVENT_MOUSE_WHEEL && !toolbox_widget->has_mouse_focus() && !layers_widget->has_mouse_focus())
    {
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
        editor->set_camera_scale(camera.get_current_scale() + wheel_y * CAMERA_ZOOM_SENSITIVITY);
      else
        editor->scroll({ static_cast<float>((m_shift_pressed ? wheel_y * (g_config->editor_invert_shift_scroll ? -1 : 1) : wheel_x) * 40),
                         static_cast<float>((m_shift_pressed ? wheel_x : wheel_y) * -40) });
    }
  }
}

void
EditorEventHandling::reset_state()
{
  m_ctrl_pressed = m_alt_pressed = false;

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
  if (!editor->has_focus())
    return;

  auto scroll_speed = editor->get_scroll_speed();

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

  if (controller.hold(Control::LEFT) || keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
    editor->scroll({ -scroll_speed, 0.0f });
  }

  if (controller.hold(Control::RIGHT) || keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
    editor->scroll({ scroll_speed, 0.0f });
  }

  if (controller.hold(Control::UP) || keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
    editor->scroll({ 0.0f, -scroll_speed });
  }

  if (controller.hold(Control::DOWN) || keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
    editor->scroll({ 0.0f, scroll_speed });
  }
}