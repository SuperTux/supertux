//  SuperTux - Add-on Manager
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
//                2014 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_ADDON_ADDON_MANAGER_HPP
#define HEADER_SUPERTUX_ADDON_ADDON_MANAGER_HPP

#ifndef __EMSCRIPTEN__

#include <memory>
#include <string>
#include <vector>

#include "addon/downloader.hpp"
#include "supertux/gameconfig.hpp"
#include "util/currenton.hpp"

class Addon;
using TransferStatusPtr = std::shared_ptr<TransferStatus>;

typedef std::string AddonId;

/** Checks for, installs and removes Add-ons */
class AddonManager final : public Currenton<AddonManager>
{
public:
  using AddonList = std::vector<std::unique_ptr<Addon> >;

private:
  Downloader m_downloader;
  std::string m_addon_directory;
  std::string m_repository_url;
  std::vector<Config::Addon>& m_addon_config;

  AddonList m_installed_addons;
  AddonList m_repository_addons;

  bool m_has_been_updated;

  TransferStatusPtr m_transfer_status;

public:
  AddonManager(const std::string& addon_directory,
               std::vector<Config::Addon>& addon_config);
  ~AddonManager();

  bool has_online_support() const;
  bool has_been_updated() const;
  void check_online();
  TransferStatusPtr request_check_online();

  std::vector<AddonId> get_repository_addons() const;
  std::vector<AddonId> get_installed_addons() const;

  Addon& get_repository_addon(const AddonId& addon) const;
  Addon& get_installed_addon(const AddonId& addon) const;

  TransferStatusPtr request_install_addon(const AddonId& addon_id);
  void install_addon(const AddonId& addon_id);
  void uninstall_addon(const AddonId& addon_id);

  void enable_addon(const AddonId& addon_id);
  void disable_addon(const AddonId& addon_id);

  bool is_old_enabled_addon(const std::unique_ptr<Addon>& addon) const;
  bool is_old_addon_enabled() const;
  void disable_old_addons();
  void mount_old_addons();
  void unmount_old_addons();
  bool is_from_old_addon(const std::string& filename) const;
  bool is_addon_installed(const std::string& id) const;

  void update();
  void check_for_langpack_updates();

private:
  std::vector<std::string> scan_for_archives() const;
  void add_installed_addons();
  AddonList parse_addon_infos(const std::string& filename) const;

  /** add \a archive, given as physfs path, to the list of installed
      archives */
  void add_installed_archive(const std::string& archive, const std::string& md5);

  /** search for an .nfo file in the top level directory that
      originates from \a archive, \a archive is a OS path */
  std::string scan_for_info(const std::string& archive) const;

private:
  AddonManager(const AddonManager&) = delete;
  AddonManager& operator=(const AddonManager&) = delete;
};

#endif

#endif

/* EOF */
