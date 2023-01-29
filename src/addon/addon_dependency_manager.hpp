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

#ifndef HEADER_SUPERTUX_ADDON_ADDON_DEPENDENCY_MANAGER_HPP
#define HEADER_SUPERTUX_ADDON_ADDON_DEPENDENCY_MANAGER_HPP

#include "addon/addon_component_manager.hpp"

#include <string>
#include <vector>

#include "addon/addon_manager.hpp"

/** Manages the download process of Add-on dependencies */
class AddonDependencyManager final : public AddonComponentManager
{
private:
  const AddonId m_addon_id;
  std::vector<std::string> m_dependencies;
  TransferStatusPtr m_transfer_status;
  std::function<void ()> m_callback;

public:
  AddonDependencyManager(const AddonId& addon_id);
  ~AddonDependencyManager() override;

  void update() override;

  void request_download_all(const std::function<void ()>& callback = {});

private:
  void request_download();

private:
  AddonDependencyManager(const AddonDependencyManager&) = delete;
  AddonDependencyManager& operator=(const AddonDependencyManager&) = delete;
};

#endif

/* EOF */
