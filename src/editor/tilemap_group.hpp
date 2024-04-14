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

#ifndef HEADER_SUPERTUX_EDITOR_TILEMAP_GROUP_HPP
#define HEADER_SUPERTUX_EDITOR_TILEMAP_GROUP_HPP

#include "editor/object_icon.hpp"

#include <memory>
#include <vector>

class LayerIcon;

class TileMapGroup final : public ObjectIcon
{
public:
  TileMapGroup(const std::string& name);

  void add(std::unique_ptr<LayerIcon> layer);
  //std::unique_ptr<LayerIcon> release(TileMap* tilemap);

  std::string get_name() const { return m_name; }
  const std::vector<std::unique_ptr<LayerIcon>>& get_tilemaps() const { return m_tilemaps; }

private:
  std::string m_name;
  std::vector<std::unique_ptr<LayerIcon>> m_tilemaps;

private:
  TileMapGroup(const TileMapGroup&) = delete;
  TileMapGroup& operator=(const TileMapGroup&) = delete;
};

#endif

/* EOF */
