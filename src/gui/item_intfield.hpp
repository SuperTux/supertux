//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
//                2022 Vankata453
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

#ifndef HEADER_SUPERTUX_GUI_ITEM_INTFIELD_HPP
#define HEADER_SUPERTUX_GUI_ITEM_INTFIELD_HPP

#include "gui/item_textfield.hpp"

class ItemIntField final : public ItemTextField
{
public:
  ItemIntField(const std::string& text_, int* input_, int id_ = -1);
  ~ItemIntField() override;

  int* number;

  /** Processes the given custom event cases. */
  virtual bool custom_event(const SDL_Event& ev) override;

  /** Processes the menu action. */
  virtual void process_action(const MenuAction& action) override;

private:
  std::string m_input;

  void add_char(char c);

private:
  ItemIntField(const ItemIntField&) = delete;
  ItemIntField& operator=(const ItemIntField&) = delete;
};

#endif

/* EOF */
