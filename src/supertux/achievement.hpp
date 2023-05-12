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

#ifndef HEADER_SUPERTUX_SUPERTUX_ACHIEVEMENT_HPP
#define HEADER_SUPERTUX_SUPERTUX_ACHIEVEMENT_HPP

#include <string>

enum AchievementType
{
  ACHIEVEMENT_INSTANT, // The achievement is applied instantly.
  ACHIEVEMENT_LEVEL // The achievement is queued to be applied on level finish.
};

struct Achievement
{
  Achievement(std::string id_, std::string name_, std::string desc_,
              std::string icon_, AchievementType type_, float goal_) :
    id(id_),
    name(name_),
    description(desc_),
    icon(icon_),
    type(type_),
    goal(goal_),
    progress(0.0f),
    completed(false)
  {}

  const std::string id;
  const std::string name;
  const std::string description;
  const std::string icon;
  const AchievementType type;
  const float goal;

  float progress;
  bool completed; // Has indicated its completion to the user.
};

#endif

/* EOF */
