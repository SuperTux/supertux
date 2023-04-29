//  SuperTux
//  Copyright (C) 2023 Vankata453
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

#include "supertux/menu/options_select_menu.hpp"

#include "gui/menu_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/options_menu.hpp"
#include "util/gettext.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

OptionsSelectMenu::OptionsSelectMenu(bool complete_options) :
  m_complete_options(complete_options)
{
  if (m_complete_options) // Currently, locale options are only available when opened completely.
  {
    add_item(_("Locale"), "", "images/engine/editor/camera.png", OptionsMenu::LOCALE);
  }
  add_item(_("Video"), "", "images/engine/editor/camera.png", OptionsMenu::VIDEO);
  add_item(_("Audio"), "", "images/engine/editor/camera.png", OptionsMenu::AUDIO);
  add_item(_("Controls"), "", "images/engine/editor/camera.png", OptionsMenu::CONTROLS);
  add_item(_("Extras"), "", "images/engine/editor/camera.png", OptionsMenu::EXTRAS);
  add_item(_("Advanced"), "", "images/engine/editor/camera.png", OptionsMenu::ADVANCED);
}

void
OptionsSelectMenu::menu_action(const OptionsSelectMenu::Item& item)
{
  MenuManager::instance().push_menu(std::make_unique<OptionsMenu>(static_cast<OptionsMenu::Type>(item.id), m_complete_options));
}


float
OptionsSelectMenu::get_y() const
{
  return static_cast<float>(SCREEN_HEIGHT) / 2 - 35.0f;
}

/* EOF */
