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

#include "gui/achievement_notification.hpp"

#include "audio/sound_manager.hpp"
#include "util/gettext.hpp"

const float AchievementNotification::s_fade_time = 0.5f;

AchievementNotification::AchievementNotification(std::string id) :
  Notification("achievement_" + id, false, true),
  m_transition_timer(),
  m_fading_out(false),
  m_hide_timer()
{
  set_mini_text(_("Achievement completed!")); // Set default mini text.

  m_transition_timer.start(s_fade_time);
  m_hide_timer.start(5);

  SoundManager::current()->preload("sounds/tada.ogg");
  SoundManager::current()->play("sounds/tada.ogg");
}

void
AchievementNotification::draw(DrawingContext& context)
{
  m_pos = Vector(static_cast<float>(context.get_width()) / 2 - std::max(m_text_size.width, m_mini_text_size.width) / 2,
                 static_cast<float>(context.get_height() / 12) - m_text_size.height - m_mini_text_size.height + 10.0f);

  // Show transition when showing and hiding.
  const float timeleft = m_transition_timer.get_timeleft();
  if (timeleft > 0 || m_fading_out)
    m_pos.y -= m_size.height * (m_fading_out ? s_fade_time - timeleft : timeleft) * 4;

  if (timeleft <= 0 && m_fading_out)
    close();

  Notification::draw(context);

  if (m_hide_timer.check())
  {
    m_fading_out = true;
    m_transition_timer.start(s_fade_time);
  }
}

/* EOF */
