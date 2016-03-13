//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_SECTOR_PARSER_HPP
#define HEADER_SUPERTUX_SUPERTUX_SECTOR_PARSER_HPP

#include <memory>
#include <string>

#include "supertux/game_object_ptr.hpp"

class Level;
class ReaderMapping;
class Sector;

class SectorParser
{
public:
  static std::unique_ptr<Sector> from_reader(Level& level, const ReaderMapping& sector);
  static std::unique_ptr<Sector> from_reader_old_format(Level& level, const ReaderMapping& sector);
  static std::unique_ptr<Sector> from_nothing(Level& level);

private:
  SectorParser(Sector& sector);

  void fix_old_tiles();
  void parse_old_format(const ReaderMapping& reader);
  void parse(const ReaderMapping& sector);
  void create_sector();
  GameObjectPtr parse_object(const std::string& name_, const ReaderMapping& reader);

private:
  Sector& m_sector;

private:
  SectorParser(const SectorParser&) = delete;
  SectorParser& operator=(const SectorParser&) = delete;
};

#endif

/* EOF */
