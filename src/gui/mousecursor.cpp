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

#include "gui/mousecursor.hpp"

#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_mouse.h>
#include <memory>

#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "sprite/sprite.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/sdl_surface.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

MouseCursor* MouseCursor::current_ = nullptr;

MouseCursor::MouseCursor(SpritePtr sprite) :
  m_state(MouseCursorState::NORMAL),
  m_applied_state(MouseCursorState::HIDE),
  m_sprite(std::move(sprite)),
  m_custom_cursor_last(false),
  m_visible(true),
  m_visibility_changed(false),
  m_cursors(),
  m_x(),
  m_y(),
  m_mobile_mode(false),
  m_icon()
{
}

void
MouseCursor::set_cursor_action(const std::string& action)
{
  if (!g_config->custom_system_cursor)
  {
    m_sprite->set_action(action);
    return;
  }

  if (m_cursors.find(action) == m_cursors.end())
  {
    auto surfaces = m_sprite->get_action_surfaces(action);
    if (surfaces->empty())
      return;
    std::string filename = (*surfaces)[0]->get_filename();
    SDLSurfacePtr surface = SDLSurface::from_file(filename);
    m_cursors[action] =
      std::move(std::shared_ptr<SDL_Cursor>(SDL_CreateColorCursor(surface.get(), 0, 0), &SDL_FreeCursor));
    if (m_cursors[action])
    {
      SDL_SetCursor(m_cursors[action].get());
    }
    else
    {
      log_warning << "Couldn't load cursor: " << SDL_GetError() << "\nRendering cursor instead.\n";
      g_config->custom_system_cursor = false;
    }
  }
  else
  {
    SDL_SetCursor(m_cursors[action].get());
  }
}

void
MouseCursor::apply_state(MouseCursorState state)
{
  // we still set the state but ensure mouse is hidden in this case.
  if (!m_visible)
  {
    if (!(g_config->custom_mouse_cursor) || g_config->custom_system_cursor)
      SDL_ShowCursor(SDL_DISABLE);
    m_applied_state = state;
    return;
  }

  if (m_applied_state != state || m_visibility_changed)
  {
    m_applied_state = state;
    m_visibility_changed = false;

    switch(state)
    {
      case MouseCursorState::NORMAL:
        set_cursor_action("default");
        break;

      case MouseCursorState::CLICK:
        set_cursor_action("click");
        break;

      case MouseCursorState::LINK:
        set_cursor_action("link");
        break;

      case MouseCursorState::HIDE:
        if (g_config->custom_system_cursor)
          SDL_ShowCursor(SDL_DISABLE);
        break;
    }
  }
}

// NOTE: visibility is not the same as MouseCursorState::HIDE, as for custom
// mouse cursors in some cases, the draw function for the MouseCursor simply
// wasn't called for the cursor. This unfortunately did nothing for native mouse
// cursors, so we can use this to be absolutely sure of the cursors state,
// regardless if it was set to ::HIDE or ::LINK or whatever
void
MouseCursor::set_visible(bool visibility)
{
  if (m_visible == visibility)
    return;
  m_visible = visibility;
  m_visibility_changed = true;
  if (m_visible && (!(g_config->custom_mouse_cursor) || g_config->custom_system_cursor))
    SDL_ShowCursor(SDL_ENABLE);

  apply_state(m_applied_state);
}

void
MouseCursor::draw(DrawingContext& context)
{
  if (!g_config->custom_mouse_cursor)
  {
    if (m_custom_cursor_last)
    {
      SDL_Cursor* default_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
      if (default_cursor)
        SDL_SetCursor(default_cursor);
      SDL_FreeCursor(default_cursor);
      m_custom_cursor_last = false;
    }
    return;
  }

  if (m_state != MouseCursorState::HIDE && m_visible)
  {
    int x, y;
    Uint32 ispressed = SDL_GetMouseState(&x, &y);

    if (g_config->custom_system_cursor && SDL_ShowCursor(SDL_QUERY) == SDL_DISABLE)
    {
      SDL_ShowCursor(SDL_ENABLE);
    }

    if (m_mobile_mode)
    {
      x = m_x;
      y = m_y;
    }

    if (ispressed & SDL_BUTTON(1) || ispressed & SDL_BUTTON(2))
    {
      apply_state(MouseCursorState::CLICK);
    }
    else
    {
      apply_state(m_state);
    }

    Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(x, y);

    if (!g_config->custom_system_cursor)
      m_sprite->draw(context.color(), mouse_pos, LAYER_GUI + 100);

    if (m_icon) {
      context.color().draw_surface(m_icon,
                                   Vector(mouse_pos.x + static_cast<float>(m_icon->get_width()) / 2,
                                          mouse_pos.y - static_cast<float>(m_icon->get_height())),
                                   LAYER_GUI + 100);
    }
  }

  m_custom_cursor_last = g_config->custom_mouse_cursor;
}
