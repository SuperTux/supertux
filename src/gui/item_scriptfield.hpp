//  SuperTux
//  Copyright (C) 2023 Vankata453
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

#ifndef HEADER_SUPERTUX_GUI_ITEM_SCRIPTFIELD_HPP
#define HEADER_SUPERTUX_GUI_ITEM_SCRIPTFIELD_HPP

#include "gui/menu_item.hpp"

#include "interface/control_scriptbox.hpp"

/** A script field item, powered by an interface textbox control. */
class ItemScriptField final : public MenuItem
{
public:
  ItemScriptField(std::unique_ptr<ControlScriptbox> control, const Sizef& relative_size);

  void draw(DrawingContext& context, const Vector& pos, int, bool) override;
  void update(float dt_sec) override;

  void process_action(const MenuAction& action) override;
  void event(const SDL_Event& ev) override;

  void on_window_resize() override;

  int get_width() const override;
  int get_height() const override;

  bool locks_navigation() const override { return true; }
  bool locked() const override { return m_control->suggestions_active(); }
  bool select_blink() const override { return false; }
  bool overrides_cursor_state() const override { return true; }

  ControlTextbox* get_control() const { return m_control.get(); }

private:
  std::unique_ptr<ControlScriptbox> m_control;
  const Sizef m_relative_size;

private:
  ItemScriptField(const ItemScriptField&) = delete;
  ItemScriptField& operator=(const ItemScriptField&) = delete;
};

#endif

/* EOF */
