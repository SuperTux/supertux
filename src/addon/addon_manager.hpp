//  SuperTux - Add-on Manager
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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

#include <memory>
#include <string>
#include <vector>

#include "addon/downloader.hpp"
#include "util/currenton.hpp"
#include "util/reader_fwd.hpp"
#include "util/writer_fwd.hpp"

class Addon;

typedef int AddonId;

/** Checks for, installs and removes Add-ons */
class AddonManager : public Currenton<AddonManager>
{
public:
  AddonManager(const std::string& addon_directory,
               std::vector<std::string>& ignored_addon_filenames_);
  ~AddonManager();

  /** returns a list of installed Add-ons */
  const std::vector<std::unique_ptr<Addon> >& get_addons() const;

  /** Returns true if online support is available */
  bool has_online_support() const;

  /** downloads list of available Add-ons */
  void check_online();

  /** Download and install Add-on */
  void install(Addon& addon);
  /** Physically delete Add-on */
  void remove(Addon& addon);

  /** Load Add-on and mark as to be loaded automatically */
  void enable(Addon& addon);
  /** Unload Add-on and mark as not to be loaded automatically */
  void disable(Addon& addon);

  Addon& get_addon(int id);
  int get_num_addons() const { return static_cast<int>(m_addons.size()); }

  /** Loads all enabled Add-ons, i.e. adds them to the search path */
  void load_addons();

private:
  /** Add Add-on to search path */
  void load(Addon& addon);
  /** Remove Add-on from search path */
  void unload(Addon& addon);

private:
  Downloader m_downloader;
  std::string m_addon_directory;
  std::vector<std::unique_ptr<Addon> > m_addons;
  std::vector<std::string>& m_ignored_addon_filenames;

private:
  AddonManager(const AddonManager&) = delete;
  AddonManager& operator=(const AddonManager&) = delete;
};

#endif

/* EOF */
