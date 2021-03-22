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
  m_escape(false),
  m_bak_escape(false),
  m_old_up(false),
  m_old_down(false),
  m_old_left(false),
  m_old_right(false),
  m_old_jump(false),
  m_old_action(false),
  m_old_escape(false),
  m_rect_directions(16.f, -144.f, 144.f, -16.f),
  m_rect_jump(-160.f, -80.f, -96.f, -16.f),
  m_rect_action(-80.f, -80.f, -16.f, -16.f),
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

  context.color().draw_surface_scaled(m_tex_dirs, apply_corner(m_rect_directions, m_screen_width, m_screen_height), 1650);

  if (m_up)
    context.color().draw_surface_scaled(m_tex_up, apply_corner(m_rect_directions, m_screen_width, m_screen_height), 1651);
  if (m_down)
    context.color().draw_surface_scaled(m_tex_dwn, apply_corner(m_rect_directions, m_screen_width, m_screen_height), 1651);
  if (m_left)
    context.color().draw_surface_scaled(m_tex_lft, apply_corner(m_rect_directions, m_screen_width, m_screen_height), 1651);
  if (m_right)
    context.color().draw_surface_scaled(m_tex_rgt, apply_corner(m_rect_directions, m_screen_width, m_screen_height), 1651);

  context.color().draw_surface_scaled(m_action ? m_tex_btn_press : m_tex_btn, apply_corner(m_rect_action, m_screen_width, m_screen_height), 1650);
  context.color().draw_surface_scaled(m_tex_action, apply_corner(m_rect_action, m_screen_width, m_screen_height), 1651);

  context.color().draw_surface_scaled(m_jump ? m_tex_btn_press : m_tex_btn, apply_corner(m_rect_jump, m_screen_width, m_screen_height), 1650);
  context.color().draw_surface_scaled(m_tex_jump, apply_corner(m_rect_jump, m_screen_width, m_screen_height), 1651);

  context.color().draw_surface_scaled(m_bak_escape ? m_tex_btn_press : m_tex_btn, apply_corner(m_rect_escape, m_screen_width, m_screen_height), 1650);
  context.color().draw_surface_scaled(m_tex_pause, apply_corner(m_rect_escape, m_screen_width, m_screen_height).grown(-8.f), 1650);
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
  m_old_escape = m_escape;

  m_up = m_down = m_left = m_right = m_jump = m_action = m_escape = false;

  // FIXME: This assumes that 1) there is only one touchscreen and 2) SuperTux
  // fills the whole screen
  if (SDL_GetNumTouchDevices() < 1)
    return;

  SDL_TouchID device = SDL_GetTouchDevice(0);

  if (device == 0)
    throw new std::runtime_error("Error getting touchscreen info: " + std::string(SDL_GetError()));

  int num_touches = SDL_GetNumTouchFingers(device);

  // FIXME: There's some weird problem with the escape button specifically, which
  // I had to patch a weird way. If someone in the future finds a fix to handle
  // escaping on mobile properly, don't forget to remove those lines.
  if (num_touches == 0)
    m_bak_escape = false;

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

  controller.set_control(Control::UP,     m_up     || (!m_old_up     && controller.hold(Control::UP)));
  controller.set_control(Control::DOWN,   m_down   || (!m_old_down   && controller.hold(Control::DOWN)));
  controller.set_control(Control::LEFT,   m_left   || (!m_old_left   && controller.hold(Control::LEFT)));
  controller.set_control(Control::RIGHT,  m_right  || (!m_old_right  && controller.hold(Control::RIGHT)));
  controller.set_control(Control::JUMP,   m_jump   || (!m_old_jump   && controller.hold(Control::JUMP)));
  controller.set_control(Control::ACTION, m_action || (!m_old_action && controller.hold(Control::ACTION)));
  controller.set_control(Control::ESCAPE, m_escape || (!m_old_escape && controller.hold(Control::ESCAPE)));
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

  // FIXME: Why do I need an extra variable (m_bak_escape) just for this one?
  // Without it, pressing escape will toggle pressed() (not hold(), pressed())
  // every single frame, apparently
  if (apply_corner(m_rect_escape, m_screen_width, m_screen_height).contains(pos))
  {
    if (!m_bak_escape)
      m_escape = true;
    m_bak_escape = true;
  }
  else
  {
    m_bak_escape = false;
  }

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
