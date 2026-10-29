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
#include "editor/editor_event_handling.hpp"
#include "interface/control.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_object.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"

EditorPropertiesPanel::EditorPropertiesPanel() :
  m_controls()
{
}

Rectf
EditorPropertiesPanel::get_area() const
{
  return Rectf(0, 32.0f, 200.0f, SCREEN_HEIGHT / 2.f);
}

Color
EditorPropertiesPanel::get_background_color() const
{
  return g_config->editorcolor;
}

int
EditorPropertiesPanel::get_layer() const
{
  return LAYER_GUI - 6;
}

bool
EditorPropertiesPanel::is_visible() const
{
  return g_config->editor_show_properties_sidebar;
}

bool
EditorPropertiesPanel::contains_position(const Vector& pos) const
{
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
  context.color().draw_filled_rect(get_area(), get_background_color(), get_layer());
  context.color().draw_rect(get_area(), Color(0.25, 0.25, 0.25), LAYER_GUI);
  context.color().set_blur(0);

  if (m_controls.empty())
  {
    std::string text = "";
    if (Editor::current()->get_selected_object() == nullptr)
    {
      text = _("Select an object or layer to display its properties here.");
    }
    else
    {
      text = _("The currently selected object or layer has no properties.");
    }

    std::string overflow;
    auto max_width = get_area().get_width() - 10.f;
    text = Resources::editor_menu_font->wrap_to_width(text, max_width, &overflow);

    while(!overflow.empty())
    {
      text += "\n";
      text += Resources::editor_menu_font->wrap_to_width(overflow, max_width, &overflow);
    }

    context.color()
        .draw_text(Resources::editor_menu_font, text, get_area().p1() + Vector(4, 4),
                   FontAlignment::ALIGN_LEFT, LAYER_GUI);
  }

  for(const auto& control : m_controls)
  {
    control->draw(context);
  }
}

bool
EditorPropertiesPanel::has_focus() const
{
  auto has_focus = [](const std::unique_ptr<InterfaceControl> &control)
  {
    return control->has_focus();
  };
  return std::any_of(m_controls.begin(), m_controls.end(), has_focus);
}

bool
EditorPropertiesPanel::has_mouse_focus(const SDL_Event& ev, const Vector& mouse_pos) const
{
  return (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
          ev.type == SDL_EVENT_MOUSE_BUTTON_UP ||
          ev.type == SDL_EVENT_MOUSE_WHEEL) &&
         contains_position(mouse_pos);
}

bool
EditorPropertiesPanel::handle_tab_event(const SDL_Event& ev)
{
  if (ev.type != SDL_EVENT_KEY_DOWN)
    return false;

  if (ev.key.key != SDLK_TAB && ev.key.key != SDLK_LEFT_TAB)
    return false;

  if (m_controls.empty() || m_controls.size() == 1)
    return false;

  bool shift_pressed = SDL_GetModState() & SDL_KMOD_SHIFT;
  auto focused_control = std::find_if(m_controls.begin(), m_controls.end(),
    [](const auto& control) { return control->has_focus(); });
  
  if (focused_control == m_controls.end())
    return false;

  focused_control->get()->set_has_focus(false);

  if (focused_control == m_controls.end() - 1 && !shift_pressed)
    focused_control = m_controls.begin();
  else if (focused_control == m_controls.begin() && shift_pressed)
    focused_control = m_controls.end() - 1;
  else if (shift_pressed)
    focused_control--;
  else
    focused_control++;

  focused_control->get()->set_has_focus(true);
  return true;
}

bool
EditorPropertiesPanel::event(const SDL_Event& ev)
{
  if (handle_tab_event(ev))
    return true;

  for (const auto &control : m_controls)
  {
    if (!control->prevents_event_propagation())
      continue;
    
    if(control->event(ev))
      return true;
  }

  bool event_handled = false;
  for (const auto &control : m_controls)
  {
    // Events that don't propagate were handled earlier
    if (control->prevents_event_propagation())
      continue;

    control->event(ev);

    if (!event_handled)
    {
      event_handled = control->has_focus();
    }
  }

  if (!event_handled)
  {
    auto editor_events = Editor::current()->get_event_handling();
    event_handled = has_mouse_focus(ev, editor_events->get_mouse_pos());
  }

  return event_handled;
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

  if (!g_config->editor_show_properties_sidebar)
  {
    return;
  }

  if (!object || !object->has_settings())
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
