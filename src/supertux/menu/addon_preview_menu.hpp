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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_ADDON_PREVIEW_MENU_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_ADDON_PREVIEW_MENU_HPP

#include <vector>

#include "addon/addon_manager.hpp"
#include "gui/dialog.hpp"
#include "gui/menu.hpp"

class Addon;
class AddonManager;

class AddonPreviewMenu final : public Menu
{
  enum {
    MNID_SHOW_SCREENSHOTS,
    MNID_INSTALL,
    MNID_UNINSTALL,
    MNID_TOGGLE
  };

private:
  AddonManager& m_addon_manager;
  AddonScreenshotManager m_screenshot_manager;
  const Addon& m_addon;
  bool m_addon_enabled;
  const bool m_auto_install;
  const bool m_update;
  std::vector<std::string> m_screenshots;
  bool m_show_screenshots;

public:
  AddonPreviewMenu(const Addon& addon, bool auto_install = false, bool update = false);
  ~AddonPreviewMenu() override;

  void rebuild_menu();
  void menu_action(MenuItem& item) override;

  void install_addon();
  void uninstall_addon();
  void toggle_addon();

private:
  AddonPreviewMenu(const AddonPreviewMenu&) = delete;
  AddonPreviewMenu& operator=(const AddonPreviewMenu&) = delete;
};

class ScreenshotDownloadDialog final : public Dialog
{
private:
  AddonScreenshotManager& m_screenshot_manager;

public:
  ScreenshotDownloadDialog(AddonScreenshotManager& screenshot_manager, bool passive = false);
  ~ScreenshotDownloadDialog();

  void update() override;

private:
  ScreenshotDownloadDialog(const ScreenshotDownloadDialog&) = delete;
  ScreenshotDownloadDialog& operator=(const ScreenshotDownloadDialog&) = delete;
};

#endif

/* EOF */
