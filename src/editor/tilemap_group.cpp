//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#include "editor/tilemap_group.hpp"

#include "editor/layer_icon.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

TileMapGroup::TileMapGroup(const std::string& name) :
  ObjectIcon("", "images/engine/editor/move-mode0.png"),
  m_name(name),
  m_tilemaps()
{
}

void
TileMapGroup::add(std::unique_ptr<LayerIcon> layer)
{
  assert(layer->is_tilemap());

  m_tilemaps.push_back(std::move(layer));
}

/* EOF */
