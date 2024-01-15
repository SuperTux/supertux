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

#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "sprite/sprite_manager.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

MouseCursor* MouseCursor::current_ = nullptr;

MouseCursor::MouseCursor() :
  MouseCursor(SpriteManager::current()->create("images/engine/menu/mousecursor.sprite"))
{
}

MouseCursor::MouseCursor(SpritePtr sprite) :
  m_state(MouseCursorState::NORMAL),
  m_applied_state(MouseCursorState::HIDE),
  m_sprite(std::move(sprite)),
  m_x(),
  m_y(),
  m_mobile_mode(false),
  m_icon()
{
}

void
MouseCursor::set_state(MouseCursorState state)
{
  m_state = state;
}

void
MouseCursor::set_icon(SurfacePtr icon)
{
  m_icon = std::move(icon);
}

void
MouseCursor::apply_state(MouseCursorState state)
{
  if (m_applied_state != state)
  {
    m_applied_state = state;

    switch(state)
    {
      case MouseCursorState::NORMAL:
        m_sprite->set_action("normal");
        break;

      case MouseCursorState::CLICK:
        m_sprite->set_action("click");
        break;

      case MouseCursorState::LINK:
        m_sprite->set_action(g_config->custom_mouse_cursor ? "link" : "normal");
        break;

      case MouseCursorState::HIDE:
        break;
    }
  }
}

void
MouseCursor::update_state()
{
  if (m_state == MouseCursorState::HIDE)
    return;

  // The current cursor should be updated according to the mouse state.
  if (current_ == this)
  {
    const Uint32 ispressed = SDL_GetMouseState(NULL, NULL);
    if (ispressed & SDL_BUTTON(1) || ispressed & SDL_BUTTON(2))
      apply_state(MouseCursorState::CLICK);
    else
      apply_state(m_state);
  }
  else
  {
    apply_state(m_state);
  }
}

void
MouseCursor::draw(DrawingContext& context, float alpha, const std::string& overlay_text)
{
  if (!g_config->custom_mouse_cursor && current_ == this)
    return;

  if (m_state == MouseCursorState::HIDE)
    return;

  update_state();

  int x = m_x;
  int y = m_y;

  if (!m_mobile_mode && current_ == this)
    SDL_GetMouseState(&x, &y);

  Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(x, y);

  context.push_transform();
  context.transform().alpha = alpha;

  m_sprite->draw(context.color(), mouse_pos, LAYER_GUI + 100);

  if (m_icon) {
    context.color().draw_surface(m_icon,
                                 Vector(mouse_pos.x,
                                        mouse_pos.y - static_cast<float>(m_icon->get_height())),
                                 LAYER_GUI + 100);
  }

  context.pop_transform();

  if (!overlay_text.empty()) // Text to be drawn over the cursor has been provided
  {
    context.color().draw_text(Resources::small_font, overlay_text,
                              mouse_pos + Vector(m_sprite->get_current_hitbox_width() / 2,
                                                 m_sprite->get_current_hitbox_height() / 2),
                              ALIGN_CENTER, LAYER_GUI + 100, Color::WHITE);
  }
}

void
MouseCursor::parse_state(const ReaderMapping& reader)
{
  reader.get("state", reinterpret_cast<int&>(m_state));
  reader.get("pos-x", m_x);
  reader.get("pos-y", m_y);

  std::string icon_filename;
  reader.get("icon", icon_filename);
  if (!m_icon || m_icon->get_filename() != icon_filename)
    m_icon = (icon_filename.empty() ? nullptr : Surface::from_file(icon_filename));
}

void
MouseCursor::write_state(Writer& writer, const std::optional<Vector>& pos) const
{
  writer.write("state", reinterpret_cast<const int&>(m_applied_state));
  writer.write("pos-x", pos ? static_cast<int>(pos->x) : m_x);
  writer.write("pos-y", pos ? static_cast<int>(pos->y) : m_y);

  if (m_icon)
    writer.write("icon", m_icon->get_filename());
}

/* EOF */
