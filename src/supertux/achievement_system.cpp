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

#include "supertux/achievement_system.hpp"

#include "gui/achievement_notification.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/menu/achievements_menu.hpp"
#include "util/gettext.hpp"

AchievementSystem::AchievementSystem() :
  m_achievements(),
  m_level_progress()
{
  // Initialize all achievements.
  m_achievements.push_back({ "10coins", _("Collect 10 coins in a level"), _("Collect 10 coins in a level."), "images/objects/coin/coin-0.png", ACHIEVEMENT_LEVEL, 10 });
  m_achievements.push_back({ "20coins", _("Collect 20 coins"), _("Collect 20 coins."), "images/objects/coin/coin-0.png", ACHIEVEMENT_INSTANT, 20 });

  AchievementData::load(m_achievements); // Load achievement progress from current profile.
}

AchievementSystem::~AchievementSystem()
{
  AchievementData::save(m_achievements); // Save achievement progress.
}

void
AchievementSystem::reload(int profile)
{
  AchievementData::save(m_achievements, profile); // Save achievement progress to old profile.
  clear_level_progress();
  AchievementData::load(m_achievements); // Load achievement progress from current profile.
}


void
AchievementSystem::show_achievements()
{
  MenuManager::instance().set_horizontal_menu(std::make_unique<AchievementsMenu>(m_achievements));
}


const Achievement&
AchievementSystem::get(const std::string& id) const
{
  for (const auto& achievement : m_achievements)
    if (achievement.id == id)
      return achievement;

  throw std::runtime_error("Achievement with ID '" + id + "' not found.");
}

Achievement&
AchievementSystem::get_write(const std::string& id)
{
  for (auto& achievement : m_achievements)
    if (achievement.id == id)
      return achievement;

  throw std::runtime_error("Achievement with ID '" + id + "' not found.");
}

void
AchievementSystem::update(std::string id, float new_progress)
{
  Achievement& achievement = get_write(id);

  if (achievement.progress >= achievement.goal) return;

  switch (achievement.type)
  {
    case ACHIEVEMENT_INSTANT:
    {
      achievement.progress += new_progress;
      check_progress();
      break;
    }
    case ACHIEVEMENT_LEVEL:
    {
      if (m_level_progress.find(id) != m_level_progress.end())
      {
        if (achievement.progress + m_level_progress[id] < achievement.goal)
        {
          m_level_progress[id] += new_progress;
        }
      }
      else
      {
        m_level_progress.insert({ id, new_progress });
      }
      break;
    }
  }
}


void
AchievementSystem::clear_level_progress()
{
  m_level_progress.clear();
}

void
AchievementSystem::merge_progress()
{
  // Merge all temporary level progress into the permanent progress.
  for (auto& progress : m_level_progress)
    get_write(progress.first).progress += progress.second;

  m_level_progress.clear();
  check_progress();
}

void
AchievementSystem::check_progress()
{
  // Check achievement progress. Execute actions on achievement unlock.
  for (auto& achievement : m_achievements)
    if (achievement.progress >= achievement.goal)
      unlock_achievement(achievement.id); // Unlock the achievement.
}


float
AchievementSystem::get_progress(const std::string& id) const
{
  try
  {
    return get(id).progress;
  }
  catch (...)
  {
    return 0.0f;
  }
}

int
AchievementSystem::calculate_progress_percentage(const std::string& id) const
{
  try
  {
    const Achievement& achievement = get(id);

    float progress = achievement.progress;
    if (m_level_progress.find(id) != m_level_progress.end())
      progress += m_level_progress.at(id); // Append temporary level progress.

    if (progress > 100) // Just in case, make sure value is always maximum 100.
      progress = 100;

    return progress / achievement.goal * 100;
  }
  catch (...)
  {
    return 0;
  }
}


void
AchievementSystem::unlock_achievement(const std::string& id)
{
  Achievement& achievement = get_write(id);

  if (achievement.completed)
    return; // If the current achievement has alredy been completed before.

  achievement.completed = true;

  // Show a notification to indicate the unlocked achievement.
  auto notif = std::make_unique<AchievementNotification>(id);
  notif->set_text(achievement.name);
  notif->set_icon(achievement.icon);

  MenuManager::instance().set_notification(std::move(notif));
}

/* EOF */
