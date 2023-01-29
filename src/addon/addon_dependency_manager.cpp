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

#include "addon/addon_dependency_manager.hpp"

#include "addon/addon.hpp"
#include "util/log.hpp"

AddonDependencyManager::AddonDependencyManager(const AddonId& addon_id) :
  m_addon_id(addon_id),
  m_dependencies(),
  m_transfer_status(),
  m_callback([](){})
{
}

AddonDependencyManager::~AddonDependencyManager()
{
}

void
AddonDependencyManager::update()
{
  if (m_transfer_status)
    m_transfer_status->update();
}

void
AddonDependencyManager::request_download_all(const std::function<void ()>& callback)
{
  m_dependencies.clear();

  std::vector<std::string> dependencies;
  try
  {
    dependencies = AddonManager::current()->get_repository_addon(m_addon_id).get_dependencies();
  }
  catch (...)
  {
    log_warning << "Cannot download dependencies for add-on \"" << m_addon_id << "\", because it isn't available in repository." << std::endl;
    return;
  }

  if (dependencies.empty()) return;
  for (const std::string& id : dependencies)
  {
    if (AddonManager::current()->is_addon_installed(id))
      continue; // Don't attempt to install add-ons that are already installed.

    try
    {
      AddonManager::current()->get_repository_addon(id);
    }
    catch (...)
    {
      continue; // Don't attempt to install add-ons that are not available.
    }
    m_dependencies.push_back(id);
  }

  m_callback = callback;
  request_download();
}

void
AddonDependencyManager::request_download()
{
  if (m_dependencies.empty())
  {
    m_callback();
    return;
  }

  m_transfer_status = AddonManager::current()->request_install_addon(m_dependencies.at(0));
  m_transfer_status->then([this](bool success)
  {
    try
    {
      AddonManager::current()->enable_addon(m_dependencies.at(0));
    }
    catch (const std::exception& err)
    {
      log_warning << "Enabling dependency add-on failed: " << err.what() << std::endl;
    }

    m_dependencies.erase(m_dependencies.begin());
    request_download();
  });
}

/* EOF */
