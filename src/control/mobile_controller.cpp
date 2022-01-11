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

#include "control/mobile_controller.hpp"

#ifdef ENABLE_TOUCHSCREEN_SUPPORT

#include <string>

#include "SDL.h"

#include "control/controller.hpp"
#include "math/vector.hpp"
#include "supertux/globals.hpp"
#include "supertux/gameconfig.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

// Util to automatically put rectangles in their corners
static Rectf apply_corner(const Rectf& rect, int screen_width, int screen_height)
{
  Rectf r = rect;

  if (r.p1().x < 0)
    r.move(Vector(static_cast<float>(screen_width), 0));

  if (r.p1().y < 0)
    r.move(Vector(0, static_cast<float>(screen_height)));

  return r;
}

MobileController::MobileController() :
  m_up(false),
  m_down(false),
  m_left(false),
  m_right(false),
  m_jump(false),
  m_action(false),
  m_cheats(false),
  m_debug(false),
  m_escape(false),
  m_old_up(false),
  m_old_down(false),
  m_old_left(false),
  m_old_right(false),
  m_old_jump(false),
  m_old_action(false),
  m_old_cheats(false),
  m_old_debug(false),
  m_old_escape(false),
  m_rect_directions(16.f, -144.f, 144.f, -16.f),
  m_rect_jump(-160.f, -80.f, -96.f, -16.f),
  m_rect_action(-80.f, -80.f, -16.f, -16.f),
  m_rect_cheats(-160.f, 16.f, -96.f, 80.f),
  m_rect_debug(-80.f, 16.f, -16.f, 80.f),
  m_rect_escape(16.f, 16.f, 64.f, 64.f),
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
  m_screen_height()
{
}

void
MobileController::draw(DrawingContext& context)
{
  if (!g_config->mobile_controls)
    return;

  m_screen_width = context.get_width();
  m_screen_height = context.get_height();

  context.color().draw_surface_scaled(m_tex_dirs, apply_corner(m_rect_directions, m_screen_width, m_screen_height), LAYER_GUI + 99);

  if (m_up)
    context.color().draw_surface_scaled(m_tex_up, apply_corner(m_rect_directions, m_screen_width, m_screen_height), LAYER_GUI + 99);
  if (m_down)
    context.color().draw_surface_scaled(m_tex_dwn, apply_corner(m_rect_directions, m_screen_width, m_screen_height), LAYER_GUI + 99);
  if (m_left)
    context.color().draw_surface_scaled(m_tex_lft, apply_corner(m_rect_directions, m_screen_width, m_screen_height), LAYER_GUI + 99);
  if (m_right)
    context.color().draw_surface_scaled(m_tex_rgt, apply_corner(m_rect_directions, m_screen_width, m_screen_height), LAYER_GUI + 99);

  context.color().draw_surface_scaled(m_action ? m_tex_btn_press : m_tex_btn, apply_corner(m_rect_action, m_screen_width, m_screen_height), LAYER_GUI + 99);
  context.color().draw_surface_scaled(m_tex_action, apply_corner(m_rect_action, m_screen_width, m_screen_height), LAYER_GUI + 99);

  context.color().draw_surface_scaled(m_jump ? m_tex_btn_press : m_tex_btn, apply_corner(m_rect_jump, m_screen_width, m_screen_height), LAYER_GUI + 99);
  context.color().draw_surface_scaled(m_tex_jump, apply_corner(m_rect_jump, m_screen_width, m_screen_height), LAYER_GUI + 99);

  context.color().draw_surface_scaled(m_escape ? m_tex_btn_press : m_tex_btn, apply_corner(m_rect_escape, m_screen_width, m_screen_height), LAYER_GUI + 99);
  context.color().draw_surface_scaled(m_tex_pause, apply_corner(m_rect_escape, m_screen_width, m_screen_height).grown(-8.f), LAYER_GUI + 99);

  if (g_config->developer_mode)
  {
    context.color().draw_surface_scaled(m_cheats ? m_tex_btn_press : m_tex_btn, apply_corner(m_rect_cheats, m_screen_width, m_screen_height), LAYER_GUI + 99);
    context.color().draw_surface_scaled(m_tex_cheats, apply_corner(m_rect_cheats, m_screen_width, m_screen_height), LAYER_GUI + 99);

    context.color().draw_surface_scaled(m_debug ? m_tex_btn_press : m_tex_btn, apply_corner(m_rect_debug, m_screen_width, m_screen_height), LAYER_GUI + 99);
    context.color().draw_surface_scaled(m_tex_debug, apply_corner(m_rect_debug, m_screen_width, m_screen_height), LAYER_GUI + 99);
  }
}

void
MobileController::update()
{
  if (!g_config->mobile_controls)
    return;

  m_old_up = m_up;
  m_old_down = m_down;
  m_old_left = m_left;
  m_old_right = m_right;
  m_old_jump = m_jump;
  m_old_action = m_action;
  m_old_cheats = m_cheats;
  m_old_debug = m_debug;
  m_old_escape = m_escape;

  m_up = m_down = m_left = m_right = m_jump = m_action = m_cheats = m_debug = m_escape = false;

  // Allow using on-screen controls with the mouse
  int x, y;
  auto buttons = SDL_GetMouseState(&x, &y);
  if ((buttons & SDL_BUTTON_LMASK) != 0)
  {
    activate_widget_at_pos(static_cast<float>(x), static_cast<float>(y));
  }

  // FIXME: This assumes that 1) there is only one touchscreen and 2) SuperTux
  // fills the whole screen
  if (SDL_GetNumTouchDevices() < 1)
    return;

  SDL_TouchID device = SDL_GetTouchDevice(0);

  if (device == 0)
    throw new std::runtime_error("Error getting touchscreen info: " + std::string(SDL_GetError()));

  int num_touches = SDL_GetNumTouchFingers(device);

  for (int i = 0; i < num_touches; i++)
  {
    SDL_Finger* finger = SDL_GetTouchFinger(device, i);

    if (!finger)
      continue;

    activate_widget_at_pos(finger->x * float(m_screen_width), finger->y * float(m_screen_height));
  }
}

void
MobileController::apply(Controller& controller) const
{
  if (!g_config->mobile_controls)
    return;

  if (m_up != m_old_up)
    controller.set_control(Control::UP, m_up);
  if (m_down != m_old_down)
    controller.set_control(Control::DOWN, m_down);
  if (m_left != m_old_left)
    controller.set_control(Control::LEFT, m_left);
  if (m_right != m_old_right)
    controller.set_control(Control::RIGHT, m_right);
  if (m_jump != m_old_jump)
    controller.set_control(Control::JUMP, m_jump);
  if (m_action != m_old_action)
    controller.set_control(Control::ACTION, m_action);
  if (m_cheats != m_old_cheats)
    controller.set_control(Control::CHEAT_MENU, m_cheats);
  if (m_debug != m_old_debug)
    controller.set_control(Control::DEBUG_MENU, m_debug);
  if (m_escape != m_old_escape)
    controller.set_control(Control::ESCAPE, m_escape);
}

void
MobileController::activate_widget_at_pos(float x, float y)
{
  if (!g_config->mobile_controls)
    return;

  Vector pos(x, y);

  if (apply_corner(m_rect_jump, m_screen_width, m_screen_height).contains(pos))
    m_jump = true;

  if (apply_corner(m_rect_action, m_screen_width, m_screen_height).contains(pos))
    m_action = true;

  if (g_config->developer_mode)
  {
    if (apply_corner(m_rect_cheats, m_screen_width, m_screen_height).contains(pos))
      m_cheats = true;

    if (apply_corner(m_rect_debug, m_screen_width, m_screen_height).contains(pos))
      m_debug = true;
  }

  if (apply_corner(m_rect_escape, m_screen_width, m_screen_height).contains(pos))
    m_escape = true;

  Rectf applied = apply_corner(m_rect_directions, m_screen_width, m_screen_height);
  Rectf up = applied;
  up.set_bottom(up.get_bottom() - up.get_height() * 2.f / 3.f);
  if (up.contains(pos))
    m_up = true;

  Rectf down = applied;
  down.set_top(down.get_top() + down.get_height() * 2.f / 3.f);
  if (down.contains(pos))
    m_down = true;

  Rectf left = applied;
  left.set_right(left.get_right() - left.get_width() * 2.f / 3.f);
  if (left.contains(pos))
    m_left = true;

  Rectf right = applied;
  right.set_left(right.get_left() + right.get_width() * 2.f / 3.f);
  if (right.contains(pos))
    m_right = true;
}

#endif

/* EOF */
