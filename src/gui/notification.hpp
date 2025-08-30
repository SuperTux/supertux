//  SuperTux
//  Copyright (C) 2022 Vankata453
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

#pragma once

#include <SDL.h>
#include <functional>
#include <string>

#include "control/controller.hpp"
#include "math/sizef.hpp"
#include "supertux/timer.hpp"
#include "video/drawing_context.hpp"

class Notification
{
private:
  const std::string m_id;
  const float m_idle_close_time;
  const bool m_auto_close;
  const bool m_auto_disable;

  Timer m_idle_close_timer;
  float m_alpha;

  std::string m_text;
  std::string m_mini_text;
  Sizef m_text_size;
  Sizef m_mini_text_size;

  Vector m_pos;
  Sizef m_size;

  Vector m_init_mouse_click;
  Vector m_mouse_pos;
  Vector m_drag;
  float m_vel;
  bool m_dragging;
  bool m_mouse_down;
  bool m_mouse_over;
  bool m_mouse_over_sym1; // Mouse is over "Do not show again".
  bool m_mouse_over_sym2; // Mouse is over "Close".

  bool m_closing;

  bool m_quit; // Requested notification quit.

  std::function<void ()> m_callback;

public:
  Notification(const std::string& id, float idle_close_time = 0.f,
                bool no_auto_close = false, bool auto_disable = false);
  ~Notification();

  void set_text(const std::string& text);
  void set_mini_text(const std::string& text);
  inline void on_press(const std::function<void ()>& callback) { m_callback = callback; }

  void event(const SDL_Event& event);
  void process_input(const Controller& controller);
  void draw(DrawingContext& context);

  // Notification actions
  void disable();
  void close();

  // Static functions, serving as utilities
  static bool is_disabled(const std::string& id);

private:
  void calculate_size();

  Vector drag_amount(const SDL_Event& ev) const;

private:
  Notification(const Notification&) = delete;
  Notification& operator=(const Notification&) = delete;
};
