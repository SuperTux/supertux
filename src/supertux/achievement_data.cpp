//  SuperTux
//  Copyright (C) 2022 Vankata453
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

#include "supertux/achievement_data.hpp"

#include <physfs.h>

#include "physfs/util.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

void
AchievementData::load(std::vector<Achievement>& data)
{
  if (!apply(data)) // When reading data fails, only clear the current data.
    clear(data);
}

bool
AchievementData::apply(std::vector<Achievement>& data)
{
  // Load file according to profile.
  const std::string file = FileSystem::join("profile" + std::to_string(g_config->profile), "achievements.stad");

  if (!PHYSFS_exists(file.c_str()))
  {
    log_info << file << " achievement data for profile " << g_config->profile << " doesn't exist, not loading." << std::endl;
    return false;
  }
  if (physfsutil::is_directory(file))
  {
    log_info << file << " is a directory, not loading." << std::endl;
    return false;
  }

  auto doc = ReaderDocument::from_file(file);
  auto root = doc.get_root();

  if (root.get_name() != "supertux-achievements")
  {
    log_warning << "File is not a supertux-achievements file." << std::endl;
    return false;
  }

  log_debug << "Loading achievement data from " << file << std::endl;

  auto mapping = root.get_mapping();

  boost::optional<ReaderCollection> achievements_mapping;
  if (mapping.get("achievements", achievements_mapping))
  {
    for (const auto& achievement_node : achievements_mapping->get_objects())
    {
      if (achievement_node.get_name() == "achievement")
      {
        auto achievement_mapping = achievement_node.get_mapping();

        std::string id;
        float progress = 0.0f;
        bool completed = false;
        if (achievement_mapping.get("id", id) &&
            achievement_mapping.get("progress", progress) &&
            achievement_mapping.get("completed", completed))
        {
          // Set the progress to an existing achievement.
          for (auto& achievement : data)
          {
            if (achievement.id == id)
            {
              achievement.progress = progress;
              achievement.completed = completed;
            }
          }
        }
      }
      else
      {
        log_warning << "Unknown token in achievement data file: " << achievement_node.get_name() << std::endl;
      }
    }
  }
  else
  {
    return false;
  }

  return true;
}

void
AchievementData::save(const std::vector<Achievement>& data, int profile)
{
  // Save to file according to profile.
  const std::string file = FileSystem::join("profile" + std::to_string(profile < 0 ? g_config->profile : profile), "achievements.stad");

  log_debug << "Saving achievement data to " << file << std::endl;

  // Make sure the file directory exists.
  std::string directory = FileSystem::dirname(file);
  if (!PHYSFS_exists(directory.c_str()))
  {
    if (!PHYSFS_mkdir(directory.c_str()))
    {
      std::ostringstream msg;
      msg << "Couldn't create directory for achievement data '"
          << directory << "': " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
      throw std::runtime_error(msg.str());
    }
  }
  if (!physfsutil::is_directory(directory))
  {
    std::ostringstream msg;
    msg << "Achievement data path '" << directory << "' is not a directory.";
    throw std::runtime_error(msg.str());
  }

  Writer writer(file);

  writer.start_list("supertux-achievements");
  writer.write("version", 1);

  writer.start_list("achievements");
  for (const auto& achievement : data)
  {
    writer.start_list("achievement");
    writer.write("id", achievement.id);
    writer.write("progress", achievement.progress);
    writer.write("completed", achievement.completed);
    writer.end_list("achievement");
  }
  writer.end_list("achievements");

  writer.end_list("supertux-achievements");
}

void
AchievementData::clear(std::vector<Achievement>& data)
{
  for (auto& achievement : data)
  {
    achievement.progress = 0.0f;
    achievement.completed = false;
  }
}

/* EOF */
