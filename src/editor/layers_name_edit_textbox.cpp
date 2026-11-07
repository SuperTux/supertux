//  SuperTux
//  Copyright (C) 2026 Tobias Markus
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

#include "editor/layers_list_item.hpp"
#include "editor/layers_name_edit_textbox.hpp"

LayersNameEditTextbox::LayersNameEditTextbox(LayersListItem* parent) :
  ControlTextbox(),
  m_parent(parent)
{
}

void LayersNameEditTextbox::on_focus()
{
  ControlTextbox::on_focus();
}

void LayersNameEditTextbox::on_blur()
{
  ControlTextbox::on_blur();
  m_parent->hide_textbox();
}