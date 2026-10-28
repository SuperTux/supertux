//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@googlemail.com>
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

#include "editor/layer_object.hpp"

#include "editor/editor.hpp"
#include "editor/layers_widget.hpp"
#include "supertux/game_object.hpp"
#include "util/reader.hpp"

void
LayerObject::remove_me()
{
  GameObject::remove_me();

  if (Editor::current()->is_active())
  {
    auto layers_widget = Editor::current()->get_layers_widget();
    layers_widget->on_layer_list_changed();
  }
}