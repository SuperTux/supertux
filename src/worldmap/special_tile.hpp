//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmail.com>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#pragma once

#include "worldmap/worldmap_object.hpp"

#include <string>

namespace worldmap {

class SpecialTile final : public WorldMapObject
{
public:
  static void register_class(ssq::VM& vm);

  enum SpecialTileDirection
  {
    NORTH = 1,
    EAST = 1<<1,
    SOUTH = 1<<2,
    WEST = 1<<3,
  };

public:
  SpecialTile(const ReaderMapping& mapping);
  ~SpecialTile() override;

  static std::string class_name() { return "special-tile"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_exposed_class_name() const override { return "SpecialTile"; }
  static std::string display_name() { return _("Special Tile"); }
  virtual std::string get_display_name() const override { return display_name(); }
  virtual GameObjectClasses get_class_types() const override { return WorldMapObject::get_class_types().add(typeid(SpecialTile)); }

  virtual void draw_worldmap(DrawingContext& context) override;

  virtual ObjectSettings get_settings() override;

  inline const std::string& get_map_message() const { return m_map_message; }
  inline bool is_passive_message() const { return m_passive_message; }
  inline const std::string& get_script() const { return m_script; }

  inline bool get_apply_action_north() const { return m_apply_action_north; }
  bool get_apply_action_east() const  { return m_apply_action_east; }
  inline bool get_apply_action_south() const { return m_apply_action_south; }
  inline bool get_apply_action_west() const { return m_apply_action_west; }

  inline bool get_blocking() const { return m_blocking; }
  inline void set_blocking(bool blocking) { m_blocking = blocking; }
  void set_direction_mask(uint8_t mask);

  uint8_t get_direction_mask() const;

private:
  /** Message to show in the Map */
  std::string m_map_message;
  bool m_passive_message;

  /** Script to execute when tile is touched */
  std::string m_script;

  /** Hide special tile */
  bool m_invisible;

  /** Blocking the path */
  bool m_blocking;

  /** Only applies actions (ie. passive messages) when going to that direction */
  std::string m_apply_direction;
  bool m_apply_action_north;
  bool m_apply_action_east;
  bool m_apply_action_south;
  bool m_apply_action_west;

private:
  SpecialTile(const SpecialTile&) = delete;
  SpecialTile& operator=(const SpecialTile&) = delete;
};

} // namespace worldmap
