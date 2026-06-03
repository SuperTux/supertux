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

#include "worldmap/level_tile.hpp"

#include <iomanip>
#include <physfs.h>
#include <sstream>

#include "editor/editor.hpp"
#include "supertux/level_parser.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "worldmap/worldmap.hpp"

namespace worldmap {

LevelTile::LevelTile(const ReaderMapping& mapping) :
  WorldMapObject(mapping, "images/worldmap/common/leveldot.sprite"),
  m_basedir(WorldMap::current() ? WorldMap::current()->get_levels_path() : ""),
  m_level_filename(),
  m_title(),
  m_auto_play(false),
  m_target_time(),
  m_extro_script(),
  m_solved(false),
  m_perfect(false),
  m_statistics(),
  m_title_color(WorldMap::s_level_title_color)
{
  if (m_basedir.empty() && Editor::current() && Editor::current()->get_world())
    m_basedir = Editor::current()->get_world()->get_basedir();

  if (!mapping.get("level", m_level_filename)) {
    // Hack for backward compatibility with 0.5.x level
    m_level_filename = m_name;
  }

  mapping.get("auto-play", m_auto_play);
  mapping.get("extro-script", m_extro_script);

  std::vector<float> vColor;
  if (mapping.get("color", vColor))
    m_title_color = Color(vColor);

  if (m_basedir == "./")
    m_basedir = "";

  if (!PHYSFS_exists(FileSystem::join(m_basedir, m_level_filename).c_str()))
  {
    log_warning << "level file '" << m_level_filename
                << "' does not exist and will not be added to the worldmap" << std::endl;
    return;
  }

  if (in_worldmap())
    load_level_information();

  set_action("default");
}

LevelTile::~LevelTile()
{
}

namespace {

// version tag on the serialised ghost path so records without it are discarded
const char* const GHOST_RUN_VERSION = "v2";

std::string serialize_ghost_run(const std::vector<LevelTile::GhostRunPoint>& path)
{
  std::ostringstream out;
  out << GHOST_RUN_VERSION;
  out << std::fixed << std::setprecision(4);

  for (const auto& point : path)
    out << ' ' << point.time << ' ' << point.position.x << ' ' << point.position.y << ' ' << point.action;

  return out.str();
}

std::vector<LevelTile::GhostRunPoint> deserialize_ghost_run(const std::string& data)
{
  std::vector<LevelTile::GhostRunPoint> path;
  std::istringstream in(data);

  std::string version;
  if (!(in >> version) || version != GHOST_RUN_VERSION)
    return path;

  float time, x, y;
  std::string action;
  while (in >> time >> x >> y >> action)
    path.push_back({time, Vector(x, y), action});

  return path;
}

} // namespace

void
LevelTile::set_best_ghost_run(const std::vector<GhostRunPoint>& best_ghost_run)
{
  m_best_ghost_run = best_ghost_run;
}

void
LevelTile::serialize_best_ghost_run(ssq::Table& table) const
{
  if (m_best_ghost_run.empty())
    return;

  table.set("ghost-path", serialize_ghost_run(m_best_ghost_run));
}

void
LevelTile::unserialize_best_ghost_run(const ssq::Table& table)
{
  std::string path_data;
  if (!table.get("ghost-path", path_data))
    return;

  m_best_ghost_run = deserialize_ghost_run(path_data);
}

void
LevelTile::load_level_information()
{
  /** Set default properties. */
  m_title = _("<no title>");
  m_target_time = 0.0f;

  if (!WorldMap::current())
    return;

  try
  {
    // Determine the level filename.
    const std::string& levels_path = WorldMap::current()->get_levels_path();
    std::string filename = levels_path + get_level_filename();

    if (levels_path == "./")
      filename = get_level_filename();

    try
    {
      auto doc = ReaderDocument::from_file(filename);
      auto root = doc.get_root();

      if (root.get_name() != "supertux-level")
        throw std::runtime_error("'" + filename + "': file is not a supertux-level file.");

      auto mapping = root.get_mapping();

      mapping.get("name", m_title);
      mapping.get("target-time", m_target_time);

      std::optional<ReaderMapping> level_stat_preferences;
      if (mapping.get("statistics", level_stat_preferences))
        m_statistics.get_preferences().parse(*level_stat_preferences);
    }
    catch (const std::exception& err)
    {
      std::stringstream out;
      out << "Cannot read level info: " << err.what() << std::endl;
      throw std::runtime_error(out.str());
    }
  }
  catch (const std::exception& err)
  {
    log_warning << "Problem when reading level information: " << err.what() << std::endl;
    return;
  }
}

void
LevelTile::update_sprite_action()
{
  if (!m_solved)
    m_sprite->set_action("default");
  else
    m_sprite->set_action((m_sprite->has_action("perfect") && m_perfect) ? "perfect" : "solved");
}

ObjectSettings
LevelTile::get_settings()
{
  // FIXME: hack to make the basedir absolute, making
  // World::get_basedir() itself absolute would be correct, but
  // invalidate savefiles.
  std::string basedir = m_basedir;
  if (!basedir.empty() && basedir.front() != '/')
    basedir = "/" + basedir;

  ObjectSettings result = WorldMapObject::get_settings();

  result.add_level(_("Level"), &m_level_filename, "level", basedir);
  result.add_script(get_uid(), _("Outro script"), &m_extro_script, "extro-script");
  result.add_bool(_("Auto play"), &m_auto_play, "auto-play", false);
  result.add_color(_("Title colour"), &m_title_color, "color", Color::WHITE);

  result.reorder({"name", "sprite", "x", "y"});

  return result;
}

} // namespace worldmap
