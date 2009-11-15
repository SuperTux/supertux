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

#include <string>
#include <vector>
namespace lisp {
class Lisp;
class Writer;
}

class Addon;

/**
 * Checks for, installs and removes Add-ons
 */
class AddonManager
{
public:
  /**
   * returns a list of installed Add-ons
   */
  std::vector<Addon*> get_addons();

  /**
   * downloads list of available Add-ons
   */
  void check_online();

  /**
   * Download and install Add-on
   */
  void install(Addon* addon);

  /**
   * Physically delete Add-on
   */
  void remove(Addon* addon);

  /**
   * Unload Add-on and mark as not to be loaded automatically
   */
  void disable(Addon* addon);

  /**
   * Load Add-on and mark as to be loaded automatically
   */
  void enable(Addon* addon);

  /**
   * Remove Add-on from search path
   */
  void unload(Addon* addon);

  /**
   * Add Add-on to search path
   */
  void load(Addon* addon);

  /**
   * Loads all enabled Add-ons, i.e. adds them to the search path
   */
  void load_addons();

  /**
   * Returns the shared AddonManager instance
   */
  static AddonManager& get_instance();

  /**
   * Write AddonManager configuration to Lisp
   */
  void write(lisp::Writer& writer);

  /**
   * Read AddonManager configuration from Lisp
   */
  void read(const lisp::Lisp& lisp);

protected:
  std::vector<Addon*> addons;
  std::vector<std::string> ignored_addon_filenames;

  AddonManager();
  ~AddonManager();
};

#endif
