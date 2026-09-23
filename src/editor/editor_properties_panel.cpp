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

#include "editor/editor_properties_panel.hpp"

#include "editor/editor.hpp"
#include "interface/control.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_object.hpp"
#include "video/color.hpp"

EditorPropertiesPanel::EditorPropertiesPanel() :
  m_controls()
{
}

Rectf
EditorPropertiesPanel::get_area() const
{
  return Rectf(0, 32.0f, 200.0f, SCREEN_HEIGHT - 32.0f);
}

Color
EditorPropertiesPanel::get_background_color() const
{
  return Color(0.2f, 0.2f, 0.2f, 0.5f);
}

int
EditorPropertiesPanel::get_layer() const
{
  return LAYER_GUI - 6;
}

bool
EditorPropertiesPanel::is_visible() const
{
  return !m_controls.empty() && g_config->editor_show_properties_sidebar;
}

bool
EditorPropertiesPanel::contains_position(const Vector& pos) const
{
  if (m_controls.empty())
    return false;
  
  return get_area().contains(pos);
}

void
EditorPropertiesPanel::update(float dt_sec)
{
  for(const auto& control : m_controls)
  {
    control->update(dt_sec);
  }
}

void
EditorPropertiesPanel::draw(DrawingContext& context)
{
  if (!is_visible())
  {
    return;
  }

  context.color().set_blur(g_config->editor_blur);
  context.color().draw_filled_rect(Rectf(0.0f, 0.0f, SCREEN_WIDTH, 32.0f),
                    get_background_color(), get_layer());

  context.color().draw_filled_rect(get_area(), get_background_color(), get_layer());
  context.color().set_blur(0);

  for(const auto& control : m_controls)
  {
    control->draw(context);
  }
}

bool
EditorPropertiesPanel::has_mouse_focus(const SDL_Event& ev, const Vector& mouse_pos) const
{
  return (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
          ev.type == SDL_EVENT_MOUSE_BUTTON_UP ||
          ev.type == SDL_EVENT_MOUSE_WHEEL ||
          ev.type == SDL_EVENT_MOUSE_MOTION) &&
         contains_position(mouse_pos);
}

bool
EditorPropertiesPanel::event(const SDL_Event& ev)
{
  for(const auto& control : m_controls)
  {
    control->event(ev);
    if (control->has_focus())
      return true;
  }

  return false;
}

void
EditorPropertiesPanel::add_control(const std::string& name, std::unique_ptr<InterfaceControl> new_control, const std::string& description)
{
  assert(new_control);
  if (!g_config->editor_show_properties_sidebar)
    return;

  float height = 35.f;
  for (const auto& control : m_controls)
    height = std::max(height, control->get_rect().get_bottom() + 5.f);

  auto control_rect = new_control->get_rect();
  Rectf target_rect;
  if (control_rect.get_width() == 0.f || control_rect.get_height() == 0.f)
  {
    target_rect = Rectf(100.f, height, 200.f - 1.0f, height + 20.f);
  }
  else
  {
    target_rect = Rectf(control_rect.get_left(), height,
                        control_rect.get_right(), height + control_rect.get_height());
  }
  new_control->set_rect(target_rect);

  auto dimensions = Rectf(3.f, height, 100.f, height + 20.f);
  new_control->m_label = std::make_unique<InterfaceLabel>(dimensions, std::move(name), std::move(description));
  m_controls.push_back(std::move(new_control));
}

void
EditorPropertiesPanel::load_object_properties(GameObject* object)
{
  m_controls.clear();

  if (!object || !g_config->editor_show_properties_sidebar)
  {
    return;
  }

  ObjectSettings os = object->get_settings();
  for (const auto& option : os.get_options())
  {
    if ((option->get_flags() & OPTION_HIDDEN) && !(option->get_flags() & OPTION_VISIBLE_PROPERTIES))
      continue;

    auto control = option->create_interface_control();
    if (!control)
      continue;

    control->m_on_activate_callbacks.emplace_back([object]() {
        object->save_state();
      });
    control->m_on_change_callbacks.emplace_back([object]() {
        // TODO: Updating the object doesn't work every time.
        // Investigate why this is the case!
        object->after_editor_set();
        object->check_state();
      });
    add_control(option->get_text(), std::move(control), option->get_description());
  }
}
