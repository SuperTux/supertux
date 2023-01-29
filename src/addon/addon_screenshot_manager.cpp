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

#include "addon/addon_screenshot_manager.hpp"

#include <physfs.h>

#include "addon/addon.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"

AddonScreenshotManager::AddonScreenshotManager(const AddonId& addon_id) :
  m_addon_manager(*AddonManager::current()),
  m_downloader(),
  m_cache_directory(),
  m_addon_id(addon_id),
  m_screenshot_urls(),
  m_local_screenshot_urls(),
  m_transfer_status(),
  m_callback([](ScreenshotList){})
{
  m_cache_directory = m_addon_manager.get_cache_directory();
  try
  {
    m_screenshot_urls = m_addon_manager.get_repository_addon(m_addon_id).get_screenshots();
  }
  catch (...)
  {
    log_warning << "Screenshots for addon \"" << m_addon_id << "\" couldn't be loaded, because it isn't available in repository." << std::endl;
  }
}

AddonScreenshotManager::~AddonScreenshotManager()
{
}

void
AddonScreenshotManager::update()
{
  m_downloader.update();
}

void
AddonScreenshotManager::request_download_all(const std::function<void (ScreenshotList)>& callback)
{
  m_local_screenshot_urls.clear();
  m_callback = callback;
  request_download(0, true);
}

void
AddonScreenshotManager::request_download(const int id, bool recursive)
{
  if (id > static_cast<int>(m_screenshot_urls.size()) - 1) //If the given screenshot ID doesn't exist, do not start the download process.
  {
    log_warning << "No screenshots available for addon \"" << m_addon_id << "\"." << std::endl;
    return;
  }

  const std::string file_name = m_addon_id + "_" + std::to_string(id + 1) + FileSystem::extension(m_screenshot_urls[id]);
  const std::string install_path = FileSystem::join(m_cache_directory, file_name);

  const bool is_last_screenshot = id == static_cast<int>(m_screenshot_urls.size()) - 1;

  if (PHYSFS_exists(install_path.c_str()))
  {
    m_local_screenshot_urls.push_back(install_path);
    if (recursive)
    {
      if (!is_last_screenshot)
      {
        request_download(id + 1, true);
      }
      else
      {
        m_callback(m_local_screenshot_urls);
        m_callback = [](ScreenshotList){};
      }
    }
  }
  else
  {
    m_transfer_status = m_downloader.request_download(m_screenshot_urls[id], install_path);
    m_transfer_status->then([this, id, recursive, is_last_screenshot, install_path](bool success)
    {
      if (!success)
      {
        log_warning << "Downloading screenshot " << (id + 1) << " of addon \"" << m_addon_id << "\" failed: " << m_transfer_status->error_msg << std::endl;
        if (!PHYSFS_delete(install_path.c_str()))
        {
          log_warning << "Error deleting screenshot file, which failed to download: PHYSFS_delete failed: " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()) << std::endl;
        }
      }
      else
      {
        m_local_screenshot_urls.push_back(install_path);
      }
      if (recursive)
      {
        if (!is_last_screenshot)
        {
          request_download(id + 1, true);
        }
        else
        {
          m_callback(m_local_screenshot_urls);
          m_callback = [](ScreenshotList){};
        }
      }
    });
  }
}

/* EOF */
