//  SuperTux
//  Copyright (C) 2004-2018 Ingo Ruhnke <grumbel@gmail.com>
//                2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_WORLDMAP_WORLDMAP_STATE_HPP
#define HEADER_SUPERTUX_WORLDMAP_WORLDMAP_STATE_HPP

namespace worldmap {

class WorldMap;

class WorldMapState
{
public:
  WorldMapState(WorldMap& worldmap);

  void load_state();
  void save_state() const;

private:
  WorldMap& m_worldmap;

private:
  WorldMapState(const WorldMapState&) = delete;
  WorldMapState& operator=(const WorldMapState&) = delete;
};

} // namespace worldmap

#endif

/* EOF */
