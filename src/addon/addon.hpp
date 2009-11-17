//  SuperTux - Add-on
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

#ifndef HEADER_SUPERTUX_ADDON_ADDON_HPP
#define HEADER_SUPERTUX_ADDON_ADDON_HPP

#include <string>

#include "util/reader_fwd.hpp"
#include "util/writer_fwd.hpp"

/**
 * Represents an (available or installed) Add-on, e.g. a level set
 */
class Addon
{
public:
  std::string kind;
  std::string title;
  std::string author;
  std::string license;
  std::string http_url;
  std::string suggested_filename; /**< filename suggested by addon author, e.g. "pak0.zip" */
  std::string installed_physfs_filename; /**< PhysFS filename on disk, e.g. "pak0.zip" */
  std::string installed_absolute_filename; /**< complete path and filename on disk, e.g. "/home/sommer/.supertux2/pak0.zip" */
  std::string stored_md5;
  bool installed;
  bool loaded;

  /**
   * Get MD5, based either on installed file's contents or stored value
   */
  std::string get_md5() const;

  /**
   * Read additional information from given contents of a (supertux-addoninfo ...) block
   */
  void parse(const Reader& lisp);

  /**
   * Read additional information from given file
   */
  void parse(std::string fname);

  /**
   * Writes out Add-on metainformation to a Lisp Writer
   */
  void write(lisp::Writer& writer) const;

  /**
   * Writes out Add-on metainformation to a file
   */
  void write(std::string fname) const;

  /**
   * Checks if Add-on is the same as given one. 
   * If available, checks MD5 sum, else relies on kind, author and title alone.
   */
  bool operator==(Addon addon2) const;

protected:
  friend class AddonManager;

  mutable std::string calculated_md5;

  Addon() :
    kind(),
    title(),
    author(),
    license(),
    http_url(),
    suggested_filename(), 
    installed_physfs_filename(), 
    installed_absolute_filename(),
    stored_md5(),
    installed(),
    loaded(),
    calculated_md5()
  {};
};

#endif

/* EOF */
