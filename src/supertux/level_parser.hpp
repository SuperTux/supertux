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

#ifndef HEADER_SUPERTUX_SUPERTUX_LEVEL_PARSER_HPP
#define HEADER_SUPERTUX_SUPERTUX_LEVEL_PARSER_HPP

#include <functional>
#include <memory>
#include <string>

class Level;
class ReaderMapping;

class LevelParser
{
public:
  static std::unique_ptr<Level> from_file(const std::string& filename);
  static void from_file(const std::string& filename,
                        std::function<void(const Level& level)> callback);
  static std::unique_ptr<Level> from_nothing(const std::string& basedir);
  static std::unique_ptr<Level> from_nothing_worldmap(const std::string& basedir, const std::string& name);

private:
  LevelParser(Level& level);

  void load(const std::string& filepath);
  void load(const std::string& filepath, std::function<void(const Level& level)> callback);
  void load_old_format(const ReaderMapping& reader);
  void create(const std::string& filepath, const std::string& levelname, bool worldmap);

private:
  Level& m_level;

private:
  LevelParser(const LevelParser&) = delete;
  LevelParser& operator=(const LevelParser&) = delete;
};

#endif

/* EOF */
