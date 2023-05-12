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

#ifndef HEADER_SUPERTUX_GUI_ACHIEVEMENT_NOTIFICATION_HPP
#define HEADER_SUPERTUX_GUI_ACHIEVEMENT_NOTIFICATION_HPP

#include "gui/notification.hpp"

#include "supertux/timer.hpp"

class AchievementNotification final : public Notification
{
private:
  static const float s_fade_time;

private:
  Timer m_transition_timer;
  bool m_fading_out;

  Timer m_hide_timer;

public:
  AchievementNotification(std::string id);

  void draw(DrawingContext& context) override;

protected:
  // Overridable properties
  bool persistent() const override { return true; }
  bool uses_custom_pos() const override { return true; }
  bool interactable() const override { return false; }

private:
  AchievementNotification(const AchievementNotification&) = delete;
  AchievementNotification& operator=(const AchievementNotification&) = delete;
};

#endif

/* EOF */
