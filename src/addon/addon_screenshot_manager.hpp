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

#ifndef HEADER_SUPERTUX_ADDON_ADDON_SCREENSHOT_MANAGER_HPP
#define HEADER_SUPERTUX_ADDON_ADDON_SCREENSHOT_MANAGER_HPP

#include "addon/addon_component_manager.hpp"

#include <string>
#include <vector>

#include "addon/addon_manager.hpp"

/** Manages screenshot previews, specified for a given Add-on */
class AddonScreenshotManager final : public AddonComponentManager
{
public:
  using ScreenshotList = std::vector<std::string>;

private:
  AddonManager& m_addon_manager;
  Downloader m_downloader;
  std::string m_cache_directory;
  const AddonId m_addon_id;
  ScreenshotList m_screenshot_urls;
  ScreenshotList m_local_screenshot_urls;
  TransferStatusPtr m_transfer_status;
  std::function<void (ScreenshotList)> m_callback;

public:
  AddonScreenshotManager(const AddonId& addon_id);
  ~AddonScreenshotManager() override;

  void update() override;

  void request_download_all(const std::function<void (ScreenshotList)>& callback = {});
  void request_download(const int id, bool recursive = false);

private:
  AddonScreenshotManager(const AddonScreenshotManager&) = delete;
  AddonScreenshotManager& operator=(const AddonScreenshotManager&) = delete;
};

#endif

/* EOF */
