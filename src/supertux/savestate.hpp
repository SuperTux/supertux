//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_SAVESTATE_HPP
#define HEADER_SUPERTUX_SUPERTUX_SAVESTATE_HPP

class Sector;

#include <sstream>

/** A savestate holds the configuration of a sector at a given frame */
class Savestate final
{
public:
  Savestate();
  Savestate(Sector* sector);

  /**
   * Saves the state of the sector at the current frame.
   * Overrides any previous save.
   */
  void save();

  /**
   * Restores the sector state according to the last save done.
   */
  void restore();

  /**
   * Resets the content of the save state.
   */
  void clear();

  std::string to_string() const { return m_backup_data.str(); }

  Sector* get_sector() const { return m_sector; }
  void set_sector(Sector* sector) { m_sector = sector; }

private:
  Sector* m_sector;
  std::stringstream m_backup_data;

private:
  Savestate(const Savestate&) = delete;
  Savestate& operator=(const Savestate&) = delete;
};

#endif

/* EOF */