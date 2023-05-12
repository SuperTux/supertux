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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_ACHIEVEMENTS_MENU_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_ACHIEVEMENTS_MENU_HPP

#include "gui/horizontal_menu.hpp"

class Achievement;

class AchievementsMenu final : public HorizontalMenu
{
private:
  struct CompletableAchievement
  {
    const bool completed;
  };

private:
  std::vector<int> m_achievement_progressions;
  std::vector<CompletableAchievement> m_completable_achievements;

public:
  AchievementsMenu(const std::vector<Achievement>& achievements);

  void draw_item(DrawingContext& context, const int& index,
                 const float& pos_x, const float& text_width) override;

private:
  // Overridable properties
  bool interactable() const override { return false; }

private:
  AchievementsMenu(const AchievementsMenu&) = delete;
  AchievementsMenu& operator=(const AchievementsMenu&) = delete;
};

#endif

/* EOF */
