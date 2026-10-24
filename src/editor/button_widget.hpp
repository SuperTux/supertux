//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "editor/widget.hpp"

#include <functional>

#include "editor/editor.hpp"
#include "editor/editor_event_handling.hpp"
#include "editor/toolbar_base_widget.hpp"
#include "editor/toolbar_widget.hpp"
#include "editor/menu_list_widget.hpp"
#include "math/rectf.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/resources.hpp"

class EditorToolbarButtonWidget : public EditorToolbarBaseWidget
{
public:
  EditorToolbarButtonWidget(SpritePtr sprite, const Vector& pos, std::function<void()> m_sig_click = {}, std::optional<Sizef> sprite_size = std::nullopt);
  EditorToolbarButtonWidget(const std::string &path, const Vector &pos, std::function<void()> callback = {}, std::optional<Sizef> sprite_size = std::nullopt);
  EditorToolbarButtonWidget(const std::string &path, std::function<void()> callback = {}, const std::string &help_text = "", std::optional<Sizef> sprite_size = std::nullopt);

  virtual bool on_mouse_button_up(const SDL_MouseButtonEvent &button) override;

  virtual void draw(DrawingContext &context) override;

  void set_sprite(const std::string& path);
  void set_sprite(SpritePtr sprite);
  
  inline void set_flat(bool flat) { m_flat = flat; }
  inline bool is_flat() const { return m_flat; }

  void set_onclick_handler(const std::function<void()>& onclick_handler)
  {
    m_sig_click = onclick_handler;
  }

protected:
  SpritePtr m_sprite;
  bool m_flat;
  std::function<void()> m_sig_click;

private:
  EditorToolbarButtonWidget(const EditorToolbarButtonWidget&) = delete;
  EditorToolbarButtonWidget& operator=(const EditorToolbarButtonWidget&) = delete;
};
