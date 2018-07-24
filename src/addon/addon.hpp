//  SuperTux - Add-on
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

#ifndef HEADER_SUPERTUX_ADDON_ADDON_HPP
#define HEADER_SUPERTUX_ADDON_ADDON_HPP

#include <memory>
#include <string>

class ReaderMapping;

class Addon
{
public:
  static std::unique_ptr<Addon> parse(const ReaderMapping& lisp);
  static std::unique_ptr<Addon> parse(const std::string& fname);

  enum Type { WORLD, WORLDMAP, LEVELSET, LANGUAGEPACK };

  enum Format {
    ORIGINAL = 0,
    WITH_MOUNTPOINT = 1
  };

private:
  // fields provided by the addon.zip itself
  std::string m_id;
  int m_version;
  Type m_type;
  std::string m_title;
  std::string m_author;
  std::string m_license;
  int m_format;

  // additional fields provided for addons from an addon repository
  std::string m_url;
  std::string m_md5;

  // fields filled by the AddonManager
  std::string m_install_filename;
  bool m_enabled;

private:
  Addon();

public:
  std::string get_id() const { return m_id; }
  int get_version() const { return m_version; }
  int get_format() const { return m_format; }

  Type get_type() const { return m_type; }
  std::string get_title() const { return m_title; }
  std::string get_author() const { return m_author; }
  std::string get_license() const { return m_license; }

  std::string get_url() const { return m_url; }
  std::string get_md5() const { return m_md5; }

  std::string get_filename() const;
  std::string get_install_filename() const;

  bool is_installed() const;
  bool is_enabled() const;

  void set_install_filename(const std::string& absolute_filename, const std::string& md5);
  void set_enabled(bool v);

private:
  Addon(const Addon&) = delete;
  Addon& operator=(const Addon&) = delete;
};

#endif

/* EOF */
