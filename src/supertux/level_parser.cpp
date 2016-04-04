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

#include "supertux/level_parser.hpp"

#include <sstream>

#include "physfs/ifile_streambuf.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "supertux/sector_parser.hpp"
#include "util/reader.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

std::unique_ptr<Level>
LevelParser::from_file(const std::string& filename)
{
  std::unique_ptr<Level> level(new Level);
  LevelParser parser(*level);
  parser.load(filename);
  return level;
}

std::unique_ptr<Level>
LevelParser::from_nothing(const std::string& basedir)
{
  std::unique_ptr<Level> level(new Level);
  LevelParser parser(*level);

  // Find a free level filename
  std::string level_file;
  int num = 0;
  do {
    num++;
    level_file = basedir + "/level" + std::to_string(num) + ".stl";
  } while ( PHYSFS_exists(level_file.c_str()) );
  std::string level_name = "Level " + std::to_string(num);
  level_file = "level" + std::to_string(num) + ".stl";

  parser.create(level_file, level_name);
  return level;
}

LevelParser::LevelParser(Level& level) :
  m_level(level)
{
}

void
LevelParser::load(const std::string& filepath)
{
  try {
    m_level.filename = filepath;
    register_translation_directory(filepath);
    auto doc = ReaderDocument::parse(filepath);
    auto root = doc.get_root();

    if(root.get_name() != "supertux-level")
      throw std::runtime_error("file is not a supertux-level file.");

    auto level = root.get_mapping();

    int version = 1;
    level.get("version", version);
    if(version == 1) {
      log_info << "[" <<  filepath << "] level uses old format: version 1" << std::endl;
      load_old_format(level);
    } else if (version == 2) {
      level.get("tileset", m_level.tileset);

      level.get("name", m_level.name);
      level.get("author", m_level.author);
      level.get("contact", m_level.contact);
      level.get("license", m_level.license);
      level.get("on-menukey-script", m_level.on_menukey_script);
      level.get("target-time", m_level.target_time);

      auto iter = level.get_iter();
      while(iter.next()) {
        if (iter.get_key() == "sector") {
          auto sector = SectorParser::from_reader(m_level, iter.as_mapping());
          m_level.add_sector(std::move(sector));
        }
      }

      if (m_level.license.empty()) {
        log_warning << "[" <<  filepath << "] The level author \"" << m_level.author
                    << "\" did not specify a license for this level \""
                    << m_level.name << "\". You might not be allowed to share it."
                    << std::endl;
      }
    } else {
      log_warning << "[" <<  filepath << "] level format version " << version << " is not supported" << std::endl;
    }
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Problem when reading level '" << filepath << "': " << e.what();
    throw std::runtime_error(msg.str());
  }
}

void
LevelParser::load_old_format(const ReaderMapping& reader)
{
  reader.get("name", m_level.name);
  reader.get("author", m_level.author);

  auto sector = SectorParser::from_reader_old_format(m_level, reader);
  m_level.add_sector(std::move(sector));
}

void
LevelParser::create(const std::string& filepath, const std::string& levelname)
{
  m_level.filename = filepath;
  m_level.name = levelname;
  m_level.license = "CC-BY-SA";

  auto sector = SectorParser::from_nothing(m_level);
  *(sector->get_name_ptr()) = "main";
  m_level.add_sector(std::move(sector));
}

/* EOF */
