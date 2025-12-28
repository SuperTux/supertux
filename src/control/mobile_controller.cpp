//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#include <config.h>
#include "control/mobile_controller.hpp"

#include <string>

#include "SDL.h"

#include "gui/menu_manager.hpp"
#include "util/log.hpp"
#include "control/controller.hpp"
#include "math/vector.hpp"
#include "supertux/globals.hpp"
#include "supertux/gameconfig.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

#define CONTROL_INT(x) (static_cast<std::underlying_type_t<Control>>(Control::x))

MobileController::MobileController() :
  // note: we reuse the controls but we do not use all of these
  m_input(),
  m_input_last(),
  m_fingers(),
  m_rect_directions(16.f, -144.f, 144.f, -16.f),
  m_rect_jump(-160.f, -80.f, -96.f, -16.f),
  m_rect_action(-80.f, -80.f, -16.f, -16.f),
  m_rect_cheats(-160.f, 16.f, -96.f, 80.f),
  m_rect_debug(-80.f, 16.f, -16.f, 80.f),
  m_rect_escape({96.f, 14.f}, Sizef{48.f, 48.f}),
  m_rect_item({0.f, 0.f}, Sizef{128.f, 128.f}),
  m_draw_directions(16.f, -144.f, 144.f, -16.f),
  m_draw_jump(-160.f, -80.f, -96.f, -16.f),
  m_draw_action(-80.f, -80.f, -16.f, -16.f),
  m_draw_cheats(-160.f, 16.f, -96.f, 80.f),
  m_draw_debug(-80.f, 16.f, -16.f, 80.f),
  m_draw_escape(Vector{192.f, 14.f}, Sizef{48.f, 48.f}),
  m_tex_dirs(Surface::from_file("/images/engine/mobile/direction.png")),
  m_tex_btn(Surface::from_file("/images/engine/mobile/button.png")),
  m_tex_btn_press(Surface::from_file("/images/engine/mobile/button_press.png")),
  m_tex_pause(Surface::from_file("/images/engine/mobile/pause.png")),
  m_tex_up(Surface::from_file("/images/engine/mobile/direction_hightlight_up.png")),
  m_tex_dwn(Surface::from_file("/images/engine/mobile/direction_hightlight_down.png")),
  m_tex_lft(Surface::from_file("/images/engine/mobile/direction_hightlight_left.png")),
  m_tex_rgt(Surface::from_file("/images/engine/mobile/direction_hightlight_right.png")),
  m_tex_jump(Surface::from_file("/images/engine/mobile/jump.png")),
  m_tex_action(Surface::from_file("/images/engine/mobile/action.png")),
  m_tex_cheats(Surface::from_file("/images/engine/mobile/cheats.png")),
  m_tex_debug(Surface::from_file("/images/engine/mobile/debug.png")),
  m_screen_width(),
  m_screen_height(),
  m_mobile_controls_scale(),
  m_haptic(nullptr, SDL_HapticClose),
  m_haptic_timer(0)
{
#ifdef __ANDROID__
  SDL_InitSubSystem(SDL_INIT_HAPTIC | SDL_INIT_TIMER);
  // ifdef'd just to be safe
  m_haptic.reset(SDL_HapticOpen(0));
  if (m_haptic)
  {
    if (!SDL_HapticRumbleSupported(m_haptic.get()))
      m_haptic.reset();

    if (m_haptic && SDL_HapticRumbleInit(m_haptic.get()) != 0)
    {
      log_warning << "Haptic device at index 0 couldn't be initialized: " << SDL_GetError() << std::endl;
      m_haptic.reset();
    }
  }
#endif
}

void
MobileController::buzz()
{
  if (!m_haptic || !g_config->touch_haptic_feedback)
    return;

  if (m_haptic_timer == 0)
    SDL_HapticRumblePlay(m_haptic.get(), 0.5f, 2000);

  m_haptic_timer = SDL_AddTimer(30, [](Uint32 val, void* _data) -> Uint32 {
    MobileController* data = static_cast<MobileController*>(_data);
    SDL_HapticRumbleStop(data->m_haptic.get());
    data->m_haptic_timer = 0;
    return 0;
  }, this);
}

void
MobileController::draw(DrawingContext& context)
{
  if (!g_config->mobile_controls)
    return;

  if (m_screen_width != static_cast<int>(context.get_width()) ||
      m_screen_height != static_cast<int>(context.get_height()) ||
      m_mobile_controls_scale != g_config->m_mobile_controls_scale)
  {
    m_screen_width = static_cast<int>(context.get_width());
    m_screen_height = static_cast<int>(context.get_height());
    float width = static_cast<float>(m_screen_width);
    float height = static_cast<float>(m_screen_height);
    m_mobile_controls_scale = g_config->m_mobile_controls_scale;
    // Buttons on Android are bigger, and direction buttons are extra wide
    // Use screen height to calculate button size, because 20:9 screen ratios are common
#ifdef __ANDROID__
    const float BUTTON_SCALE = 0.4f * g_config->m_mobile_controls_scale;
#else
    const float BUTTON_SCALE = 0.2f * g_config->m_mobile_controls_scale;
#endif
    m_rect_directions.set_size(height * BUTTON_SCALE * 4 / 3, height * BUTTON_SCALE);
    m_rect_directions.set_pos(Vector(0, height - height * BUTTON_SCALE));
    m_draw_directions = Rectf::from_center(m_rect_directions.get_middle(),
      Sizef(m_rect_directions.get_height() / 2, m_rect_directions.get_height() / 2));

    m_rect_jump.set_size(height * BUTTON_SCALE, height * BUTTON_SCALE);
    m_rect_jump.set_pos(Vector(width - height * BUTTON_SCALE, height - height * BUTTON_SCALE));
    m_draw_jump = m_rect_jump.grown(-m_rect_jump.get_height() * 3 / 8);

    m_rect_action.set_size(height * BUTTON_SCALE, height * BUTTON_SCALE);
    m_rect_action.set_pos(Vector(width - 2 * height * BUTTON_SCALE, height - height * BUTTON_SCALE));
    m_draw_action = m_rect_action.grown(-m_rect_action.get_height() * 3 / 8);

    m_rect_escape.set_size(height * BUTTON_SCALE / 2, height * BUTTON_SCALE / 2);
    m_draw_escape = m_rect_escape.grown(-m_rect_escape.get_height() / 4);

    m_rect_cheats.set_size(height * BUTTON_SCALE / 2, height * BUTTON_SCALE / 2);
    m_rect_cheats.set_pos(Vector(width - 2 * height * BUTTON_SCALE / 2, 0));
    m_draw_cheats = m_rect_cheats.grown(-m_rect_cheats.get_height() / 4);

    m_rect_debug.set_size(height * BUTTON_SCALE / 2, height * BUTTON_SCALE / 2);
    m_rect_debug.set_pos(Vector(width - height * BUTTON_SCALE / 2, 0));
    m_draw_debug = m_rect_debug.grown(-m_rect_debug.get_height() / 4);
  }

  PaintStyle translucent;
  translucent.set_alpha(0.5f);

  context.color().draw_surface_scaled(m_tex_dirs, m_draw_directions, LAYER_GUI + 99, translucent);

  if (m_input[CONTROL_INT(UP)])
    context.color().draw_surface_scaled(m_tex_up, m_draw_directions, LAYER_GUI + 99, translucent);
  if (m_input[CONTROL_INT(DOWN)])
    context.color().draw_surface_scaled(m_tex_dwn, m_draw_directions, LAYER_GUI + 99, translucent);
  if (m_input[CONTROL_INT(LEFT)])
    context.color().draw_surface_scaled(m_tex_lft, m_draw_directions, LAYER_GUI + 99, translucent);
  if (m_input[CONTROL_INT(RIGHT)])
    context.color().draw_surface_scaled(m_tex_rgt, m_draw_directions, LAYER_GUI + 99, translucent);

  context.color().draw_surface_scaled(m_input[CONTROL_INT(ACTION)] ? m_tex_btn_press : m_tex_btn, m_draw_action, LAYER_GUI + 99, translucent);
  context.color().draw_surface_scaled(m_tex_action, m_draw_action, LAYER_GUI + 99, translucent);

  context.color().draw_surface_scaled(m_input[CONTROL_INT(JUMP)] ? m_tex_btn_press : m_tex_btn, m_draw_jump, LAYER_GUI + 99, translucent);
  context.color().draw_surface_scaled(m_tex_jump, m_draw_jump, LAYER_GUI + 99, translucent);

  context.color().draw_surface_scaled(m_input[CONTROL_INT(ESCAPE)] ? m_tex_btn_press : m_tex_btn, m_draw_escape, LAYER_GUI + 99, translucent);
  context.color().draw_surface_scaled(m_tex_pause, m_draw_escape.grown(-m_draw_escape.get_height() / 8), LAYER_GUI + 99, translucent);

  if (g_config->developer_mode)
  {
    context.color().draw_surface_scaled(m_input[CONTROL_INT(CHEAT_MENU)] ? m_tex_btn_press : m_tex_btn, m_draw_cheats, LAYER_GUI + 99, translucent);
    context.color().draw_surface_scaled(m_tex_cheats, m_draw_cheats, LAYER_GUI + 99, translucent);

    context.color().draw_surface_scaled(m_input[CONTROL_INT(DEBUG_MENU)] ? m_tex_btn_press : m_tex_btn, m_draw_debug, LAYER_GUI + 99, translucent);
    context.color().draw_surface_scaled(m_tex_debug, m_draw_debug, LAYER_GUI + 99, translucent);
  }
}

void
MobileController::update()
{
  if (!g_config->mobile_controls)
    return;

  m_input_last = m_input;
  // reset
  m_input.reset();

  // Allow using on-screen controls with the mouse
  int x, y;
  auto buttons = SDL_GetMouseState(&x, &y);
  if ((buttons & SDL_BUTTON_LMASK) != 0)
  {
    activate_widget_at_pos(static_cast<float>(x), static_cast<float>(y));
  }

  for (auto& i : m_fingers)
  {
    activate_widget_at_pos(i.second.x, i.second.y);
  }

  for (size_t i = 0; i < static_cast<size_t>(Control::CONTROLCOUNT); ++i)
  {
    if (m_input[i] != m_input_last[i] && m_input[i] == true)
    {
      if (g_config->touch_just_directional &&
          !(i >= CONTROL_INT(LEFT) && i < CONTROL_INT(JUMP)))
      {
        continue;
      }
      buzz();
      break;
    }
  }
}

void
MobileController::apply(Controller& controller) const
{
  if (!g_config->mobile_controls)
    return;

  for (size_t i = 0; i < static_cast<size_t>(Control::CONTROLCOUNT); ++i)
    if (m_input[i] != m_input_last[i])
      controller.set_control(static_cast<Control>(i), static_cast<bool>(m_input[i]));

  // something is pressed
  if (m_input != 0)
  {
    controller.set_touchscreen(true);
  }
}

bool
MobileController::process_finger_down_event(const SDL_TouchFingerEvent& event)
{
  Vector pos(event.x * float(m_screen_width), event.y * float(m_screen_height));
  m_fingers[event.fingerId] = pos;
  return m_rect_jump.contains(pos) ||
    m_rect_action.contains(pos) ||
    m_rect_escape.contains(pos) ||
    m_rect_item.contains(pos) ||
    m_rect_directions.contains(pos) ||
    (g_config->developer_mode && m_rect_cheats.contains(pos)) ||
    (g_config->developer_mode && m_rect_debug.contains(pos));
}

bool
MobileController::process_finger_up_event(const SDL_TouchFingerEvent& event)
{
  Vector pos(event.x * float(m_screen_width), event.y * float(m_screen_height));
  m_fingers.erase(event.fingerId);
  return m_rect_jump.contains(pos) ||
    m_rect_action.contains(pos) ||
    m_rect_escape.contains(pos) ||
    m_rect_item.contains(pos) ||
    m_rect_directions.contains(pos) ||
    (g_config->developer_mode && m_rect_cheats.contains(pos)) ||
    (g_config->developer_mode && m_rect_debug.contains(pos));
}

bool
MobileController::process_finger_motion_event(const SDL_TouchFingerEvent& event)
{
  Vector pos(event.x * float(m_screen_width), event.y * float(m_screen_height));
  m_fingers[event.fingerId] = pos;
  return m_rect_jump.contains(pos) ||
    m_rect_action.contains(pos) ||
    m_rect_escape.contains(pos) ||
    m_rect_item.contains(pos) ||
    m_rect_directions.contains(pos) ||
    (g_config->developer_mode && m_rect_cheats.contains(pos)) ||
    (g_config->developer_mode && m_rect_debug.contains(pos));
}

void
MobileController::activate_widget_at_pos(float x, float y)
{
  if (!g_config->mobile_controls)
    return;

  Vector pos(x, y);

  if (m_rect_jump.contains(pos))
    m_input.set(CONTROL_INT(JUMP), true);
  if (m_rect_action.contains(pos))
    m_input.set(CONTROL_INT(ACTION), true);
  if (m_rect_item.contains(pos))
    m_input.set(CONTROL_INT(ITEM), true);

  if (g_config->developer_mode)
  {
    if (m_rect_cheats.contains(pos))
      m_input.set(CONTROL_INT(CHEAT_MENU), true);
    if (m_rect_debug.contains(pos))
      m_input.set(CONTROL_INT(DEBUG_MENU), true);
  }

  if (m_rect_escape.contains(pos))
    m_input.set(CONTROL_INT(ESCAPE), true);

  Rectf up = m_rect_directions;
  up.set_bottom(up.get_bottom() - up.get_height() * 2.f / 3.f);
  if (up.contains(pos))
    m_input.set(CONTROL_INT(UP), true);

  Rectf down = m_rect_directions;
  down.set_top(down.get_top() + down.get_height() * 2.f / 3.f);
  if (down.contains(pos))
    m_input.set(CONTROL_INT(DOWN), true);

  Rectf left = m_rect_directions;
  left.set_right(left.get_right() - left.get_width() * 7.f / 12.f);
  if (left.contains(pos))
    m_input.set(CONTROL_INT(LEFT), true);

  Rectf right = m_rect_directions;
  right.set_left(right.get_left() + right.get_width() * 7.f / 12.f);
  if (right.contains(pos))
    m_input.set(CONTROL_INT(RIGHT), true);
}
