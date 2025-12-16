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

#pragma once

#include <limits>
#include "gui/item_textfield.hpp"
#include "item_field_range.hpp"

class ItemIntField final : public ItemTextField
{
public:
ItemIntField(const std::string& text_, int* input_, int id_ = -1, bool positive = false, ItemIntFieldRange range = {});
  ~ItemIntField() override;

  int* number;

  /** Calls when the input gets updated. */
  virtual void on_input_update() override;

  // Text manipulation and navigation functions

  virtual void insert_text(const std::string& text, const int left_offset_pos) override;

private:
  std::string m_input;
  const bool m_positive;
  ItemIntFieldRange m_range;

  void add_char(char c, const int left_offset_pos);

private:
  ItemIntField(const ItemIntField&) = delete;
  ItemIntField& operator=(const ItemIntField&) = delete;
};
