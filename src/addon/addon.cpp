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

#ifndef __EMSCRIPTEN__

#include "addon/addon.hpp"

#include <sstream>

#include "util/reader.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

namespace {

static const char* s_allowed_characters = "-_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

Addon::Type addon_type_from_string(const std::string& type)
{
  if (type == "world")
  {
    return Addon::WORLD;
  }
  else if (type == "worldmap")
  {
    return Addon::WORLDMAP;
  }
  else if (type == "levelset")
  {
    return Addon::LEVELSET;
  }
  else if (type == "languagepack")
  {
    return Addon::LANGUAGEPACK;
  }
  else if (type == "addon")
  {
    return Addon::ADDON;
  }
  else
  {
    throw std::runtime_error("not a valid Addon::Type: " + type);
  }
}

} // namespace

std::unique_ptr<Addon>
Addon::parse(const ReaderMapping& mapping)
{
  std::unique_ptr<Addon> addon(new Addon);

  try
  {
    if (!mapping.get("id", addon->m_id))
    {
      throw std::runtime_error("(id ...) field missing from addon description");
    }

    if (addon->m_id.empty())
    {
      throw std::runtime_error("addon id is empty");
    }

    if (addon->m_id.find_first_not_of(s_allowed_characters) != std::string::npos)
    {
      throw std::runtime_error("addon id contains illegal characters: " + addon->m_id);
    }

    mapping.get("version", addon->m_version);

    std::string type;
    mapping.get("type", type);
    addon->m_type = addon_type_from_string(type);

    mapping.get("title", addon->m_title);
    mapping.get("author", addon->m_author);
    mapping.get("license", addon->m_license);
    mapping.get("url", addon->m_url);
    mapping.get("md5", addon->m_md5);
    mapping.get("format", addon->m_format);

    return addon;
  }
  catch(const std::exception& err)
  {
    std::stringstream msg;
    msg << "Problem when parsing addoninfo: " << err.what();
    throw std::runtime_error(msg.str());
  }
}

std::unique_ptr<Addon>
Addon::parse(const std::string& fname)
{
  try
  {
    register_translation_directory(fname);
    auto doc = ReaderDocument::from_file(fname);
    auto root = doc.get_root();
    if (root.get_name() != "supertux-addoninfo")
    {
      throw std::runtime_error("file is not a supertux-addoninfo file.");
    }
    else
    {
      return parse(root.get_mapping());
    }
  }
  catch(const std::exception& err)
  {
    std::stringstream msg;
    msg << "Problem when reading addoninfo '" << fname << "': " << err.what();
    throw std::runtime_error(msg.str());
  }
}

Addon::Addon() :
  m_id(),
  m_version(0),
  m_type(),
  m_title(),
  m_author(),
  m_license(),
  m_format(0),
  m_url(),
  m_md5(),
  m_install_filename(),
  m_enabled(false)
{}

std::string
Addon::get_filename() const
{
  return get_id() + ".zip";
}

std::string
Addon::get_install_filename() const
{
  return m_install_filename;
}

bool
Addon::is_installed() const
{
  return !m_install_filename.empty();
}

bool
Addon::is_enabled() const
{
  return m_enabled;
}

void
Addon::set_install_filename(const std::string& absolute_filename, const std::string& md5)
{
  m_install_filename = absolute_filename;
  m_md5 = md5;
}

void
Addon::set_enabled(bool v)
{
  m_enabled = v;
}

#endif

/* EOF */
