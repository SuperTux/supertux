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

#ifndef HEADER_SUPERTUX_SUPERTUX_ACHIEVEMENT_SYSTEM_HPP
#define HEADER_SUPERTUX_SUPERTUX_ACHIEVEMENT_SYSTEM_HPP

#include "util/currenton.hpp"

#include <map>

#include "supertux/achievement_data.hpp"
#include "video/drawing_context.hpp"

/** System to track and manage achievements, according to their type. */
class AchievementSystem final : public Currenton<AchievementSystem>
{
private:
  std::vector<Achievement> m_achievements;

  // Temporarily stored progress in the current level.
  std::map<std::string, float> m_level_progress;

public:
  AchievementSystem();
  ~AchievementSystem();

  void reload(int profile = -1);
  void reset_progress();

  void show_achievements();

  const Achievement& get(const std::string& id) const;
  void update(std::string id, float new_progress);

  void clear_level_progress();
  void merge_progress();
  void check_progress();

  float get_progress(const std::string& id) const;
  int calculate_progress_percentage(const std::string& id) const;

private:
  Achievement& get_write(const std::string& id);

  void unlock_achievement(const std::string& id);

private:
  AchievementSystem(const AchievementSystem&) = delete;
  AchievementSystem& operator=(const AchievementSystem&) = delete;
};

#endif

/* EOF */
