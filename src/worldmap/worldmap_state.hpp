//  SuperTux
//  Copyright (C) 2004-2018 Ingo Ruhnke <grumbel@gmail.com>
//                2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//                2023 Vankata453
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

/** Macro to allow quick and easy access to the current Squirrel VM. **/
#define WORLDMAP_STATE_SQUIRREL_VM_GUARD                                  \
  SquirrelVM& vm = SquirrelVirtualMachine::current()->get_vm()

/** Macro to allow quick and easy access to the current WorldMapSector. **/
#define WORLDMAP_STATE_SECTOR_GUARD                                       \
  WorldMapSector& sector = m_worldmap.get_sector()


class WorldMap;

class WorldMapState
{
public:
  WorldMapState(WorldMap& worldmap);

  void load_state();
  void save_state() const;

private:
  void load_tux();
  void load_levels();
  void load_tilemap_visibility();
  void load_sprite_change_objects();

  void save_tux() const;
  void save_levels() const;
  void save_tilemap_visibility() const;
  void save_sprite_change_objects() const;

private:
  WorldMap& m_worldmap;

  /** Variables, related to loading. **/
  bool m_position_was_reset;

private:
  WorldMapState(const WorldMapState&) = delete;
  WorldMapState& operator=(const WorldMapState&) = delete;
};

} // namespace worldmap

#endif

/* EOF */
