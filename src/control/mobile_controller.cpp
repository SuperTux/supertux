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

#include <string>

#include "SDL.h"

#include "math/vector.hpp"
#include "control/controller.hpp"
#include "video/drawing_context.hpp"

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
  m_old_escape(false),
  m_rect_directions(16.f, -144.f, 144.f, -16.f),
  m_rect_jump(-160.f, -80.f, -96.f, -16.f),
  m_rect_action(-80.f, -80.f, -16.f, -16.f),
  m_rect_escape(16.f, 16.f, 64.f, 64.f),
  m_screen_width(),
  m_screen_height()
{
}

void
MobileController::draw(DrawingContext& context)
{
  m_screen_width = context.get_width();
  m_screen_height = context.get_height();

  context.color().draw_filled_rect(apply_corner(m_rect_directions, m_screen_width, m_screen_height), Color(m_left, m_down, m_right), 999.f, 1650);
  context.color().draw_filled_rect(apply_corner(m_rect_action, m_screen_width, m_screen_height), m_action ? Color::WHITE : Color::BLACK, 1650);
  context.color().draw_filled_rect(apply_corner(m_rect_jump, m_screen_width, m_screen_height), m_jump ? Color::WHITE : Color::BLACK, 1650);
  context.color().draw_filled_rect(apply_corner(m_rect_escape, m_screen_width, m_screen_height), m_escape ? Color::WHITE : Color::BLACK, 999.f, 1650);
}

void
MobileController::update()
{
  m_up = m_down = m_left = m_right = m_jump = m_action = m_escape = false;

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
  controller.set_control(Control::UP,     m_up    );
  controller.set_control(Control::DOWN,   m_down  );
  controller.set_control(Control::LEFT,   m_left  );
  controller.set_control(Control::RIGHT,  m_right );
  controller.set_control(Control::JUMP,   m_jump  );
  controller.set_control(Control::ACTION, m_action);
  controller.set_control(Control::ESCAPE, m_escape);
}

void
MobileController::activate_widget_at_pos(float x, float y)
{
  Vector pos(x, y);

  if (apply_corner(m_rect_jump, m_screen_width, m_screen_height).contains(pos))
    m_jump = true;

  if (apply_corner(m_rect_action, m_screen_width, m_screen_height).contains(pos))
    m_action = true;

  // FIXME: Why do I need an extra variable (m_old_escape) just for this one?
  // Without it, pressing escape will toggle pressed() (not hold(), pressed())
  // every single frame, apparently
  if (apply_corner(m_rect_escape, m_screen_width, m_screen_height).contains(pos))
  {
    if (!m_old_escape)
      m_escape = true;
    m_old_escape = true;
  }
  else
  {
    m_old_escape = false;
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

/* EOF */
