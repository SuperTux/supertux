//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include <SDL_mouse.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "config.h"

#include "sprite/sprite_ptr.hpp"
#include "util/currenton.hpp"
#include "video/surface_ptr.hpp"

class DrawingContext;

enum class MouseCursorState
{
  NORMAL,
  CLICK,
  LINK,
  HIDE
};

/** Mouse cursor.
    Used to create mouse cursors.
    The mouse cursors can be animated
    and can be used in four different states. */
class MouseCursor final : public Currenton<MouseCursor>
{
public:
  static MouseCursor* current() { return current_; }
  static void set_current(MouseCursor* pcursor) { current_ = pcursor; }

private:
  static MouseCursor* current_;

public:
  MouseCursor(SpritePtr sprite);

  void draw(DrawingContext& context);

  inline void set_state(MouseCursorState state) { m_state = state; }
  inline void set_icon(SurfacePtr icon) { m_icon = std::move(icon); }

  inline void set_pos(int x, int y) { m_mobile_mode = true; m_x = x; m_y = y; }
  // This is not the same thing as MouseCursorState::HIDE
  inline bool get_visible() { return m_visible; }
  void set_visible(bool visibility);

private:
  void apply_state(MouseCursorState state);
  void set_cursor_action(const std::string& action);

private:
  MouseCursorState m_state;
  MouseCursorState m_applied_state;
  SpritePtr m_sprite;
  int m_x, m_y;
  bool m_mobile_mode;
  SurfacePtr m_icon;
  bool m_custom_cursor_last;
  bool m_visible;
  bool m_visibility_changed;

  std::unordered_map<std::string, std::shared_ptr<SDL_Cursor>> m_cursors;

private:
  MouseCursor(const MouseCursor&) = delete;
  MouseCursor& operator=(const MouseCursor&) = delete;
};
