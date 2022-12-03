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

#include "supertux/menu/achievements_menu.hpp"

#include "supertux/achievement.hpp"
#include "supertux/achievement_system.hpp"
#include "supertux/resources.hpp"
#include "video/color.hpp"
#include "video/surface.hpp"

AchievementsMenu::AchievementsMenu(const std::vector<Achievement>& achievements) :
  m_achievement_progressions(),
  m_completable_achievements()
{
  const AchievementSystem* system = AchievementSystem::current();

  for (const auto& achievement : achievements)
  {
    add_item(achievement.name, achievement.description, achievement.icon);

    m_achievement_progressions.push_back(system->calculate_progress_percentage(achievement.id));
    m_completable_achievements.push_back({ achievement.completed });
  }
}

void
AchievementsMenu::draw_item(DrawingContext& context, const int& index,
                            const float& pos_x, const float& text_width)
{
  HorizontalMenu::draw_item(context, index, pos_x, text_width);

  // Draw progress percentage.
  context.color().draw_text(Resources::normal_font, std::to_string(m_achievement_progressions[index]) + "%",
                            Vector(pos_x + text_width / 2.5f + s_height / 2,
                                   get_y() + s_icon_y + s_height / 2.8f),
                            ALIGN_CENTER, LAYER_GUI, m_completable_achievements[index].completed ? Color::GREEN : Color::YELLOW);
}

/* EOF */
