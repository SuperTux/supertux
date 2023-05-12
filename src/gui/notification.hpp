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

#ifndef HEADER_SUPERTUX_GUI_NOTIFICATION_HPP
#define HEADER_SUPERTUX_GUI_NOTIFICATION_HPP

#include <SDL.h>
#include <functional>
#include <string>

#include "control/controller.hpp"
#include "math/sizef.hpp"
#include "video/drawing_context.hpp"
#include "video/surface_ptr.hpp"

class Notification
{
protected:
  static const float s_icon_size;

  static const std::string s_sym1;
  static const std::string s_sym2;

protected:
  const std::string m_id;
  const bool m_auto_hide;
  const bool m_auto_disable;

  std::string m_text;
  std::string m_mini_text;
  Sizef m_text_size;
  Sizef m_mini_text_size;

  SurfacePtr m_icon;

  Vector m_pos;
  Sizef m_size;

  Vector m_mouse_pos;
  bool m_mouse_over;
  bool m_mouse_over_sym1; // Mouse is over "Do not show again".
  bool m_mouse_over_sym2; // Mouse is over "Close".

  bool m_quit; // Requested notification quit.

  std::function<void ()> m_callback;

public:
  Notification(std::string id, bool no_auto_hide = false, bool no_auto_disable = false);
  ~Notification();

  void set_text(const std::string& text);
  void set_mini_text(const std::string& text);
  void set_icon(const std::string& file);
  void on_press(const std::function<void ()>& callback) { m_callback = callback; }

  virtual void event(const SDL_Event& event);
  virtual void process_input(const Controller& controller);
  virtual void draw(DrawingContext& context);

  // Notification actions
  void disable();
  void close();

  // Static utilities
  static bool is_disabled(std::string id);

protected:
  virtual void calculate_size();

  // Overridable properties
  virtual bool persistent() const { return false; } // Should always be visible.
  virtual bool uses_custom_pos() const { return false; } // Uses custom position.
  virtual bool interactable() const { return true; } // The user can interact.

private:
  Notification(const Notification&) = delete;
  Notification& operator=(const Notification&) = delete;
};

#endif

/* EOF */
