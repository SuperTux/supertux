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

#include "addon/addon_manager.hpp"

#include <config.h>
#include <version.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <physfs.h>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <sys/stat.h>

#include "addon/addon.hpp"
#include "addon/md5.hpp"
#include "lisp/list_iterator.hpp"
#include "lisp/parser.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "util/writer.hpp"

namespace {

MD5 md5_from_file(const std::string& filename)
{
  // TODO: this does not work as expected for some files -- IFileStream seems to not always behave like an ifstream.
  //IFileStream ifs(installed_physfs_filename);
  //std::string md5 = MD5(ifs).hex_digest();

  MD5 md5;

  PHYSFS_file* file = PHYSFS_openRead(filename.c_str());
  if (!file)
  {
    std::ostringstream out;
    out << "PHYSFS_openRead() failed: " << PHYSFS_getLastError();
    throw std::runtime_error(out.str());
  }
  else
  {
    while (true)
    {
      unsigned char buffer[1024];
      PHYSFS_sint64 len = PHYSFS_read(file, buffer, 1, sizeof(buffer));
      if (len <= 0) break;
      md5.update(buffer, len);
    }
    PHYSFS_close(file);

    return md5;
  }
}

bool has_suffix(const std::string& str, const std::string& suffix)
{
  if (str.length() >= suffix.length())
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
  else
    return false;
}

} // namespace

AddonManager::AddonManager(const std::string& addon_directory,
                           std::vector<Config::Addon>& addon_config) :
  m_downloader(),
  m_addon_directory(addon_directory),
  m_repository_url("https://raw.githubusercontent.com/SuperTuxTeam/addons/master/index-0_4_0.nfo"),
  m_addon_config(addon_config),
  m_installed_addons(),
  m_repository_addons(),
  m_has_been_updated(false),
  m_transfer_status()
{
  PHYSFS_mkdir(m_addon_directory.c_str());

  add_installed_addons();

  // FIXME: We should also restore the order here
  for(auto& addon : m_addon_config)
  {
    if (addon.enabled)
    {
      try
      {
        enable_addon(addon.id);
      }
      catch(const std::exception& err)
      {
        log_warning << "failed to enable addon from config: " << err.what() << std::endl;
      }
    }
  }

  try
  {
    m_repository_addons = parse_addon_infos("/addons/repository.nfo");
  }
  catch(const std::exception& err)
  {
    log_warning << "parsing repository.nfo failed: " << err.what() << std::endl;
  }
}

AddonManager::~AddonManager()
{
  // sync enabled/disabled addons into the config for saving
  m_addon_config.clear();
  for(auto& addon : m_installed_addons)
  {
    m_addon_config.push_back({addon->get_id(), addon->is_enabled()});
  }
}

Addon&
AddonManager::get_repository_addon(const AddonId& id)
{
  auto it = std::find_if(m_repository_addons.begin(), m_repository_addons.end(),
                         [&id](const std::unique_ptr<Addon>& addon)
                         {
                           return addon->get_id() == id;
                         });

  if (it != m_repository_addons.end())
  {
    return **it;
  }
  else
  {
    throw std::runtime_error("Couldn't find repository Addon with id: " + id);
  }
}

Addon&
AddonManager::get_installed_addon(const AddonId& id)
{
  auto it = std::find_if(m_installed_addons.begin(), m_installed_addons.end(),
                         [&id](const std::unique_ptr<Addon>& addon)
                         {
                           return addon->get_id() == id;
                         });

  if (it != m_installed_addons.end())
  {
    return **it;
  }
  else
  {
    throw std::runtime_error("Couldn't find installed Addon with id: " + id);
  }
}

std::vector<AddonId>
AddonManager::get_repository_addons() const
{
  std::vector<AddonId> results;
  results.reserve(m_repository_addons.size());
  std::transform(m_repository_addons.begin(), m_repository_addons.end(),
                 std::back_inserter(results),
                 [](const std::unique_ptr<Addon>& addon)
                 {
                   return addon->get_id();
                 });
  return results;
}


std::vector<AddonId>
AddonManager::get_installed_addons() const
{
  std::vector<AddonId> results;
  results.reserve(m_installed_addons.size());
  std::transform(m_installed_addons.begin(), m_installed_addons.end(),
                 std::back_inserter(results),
                 [](const std::unique_ptr<Addon>& addon)
                 {
                   return addon->get_id();
                 });
  return results;
}

bool
AddonManager::has_online_support() const
{
  return true;
}

bool
AddonManager::has_been_updated() const
{
  return m_has_been_updated;
}

TransferStatusPtr
AddonManager::request_check_online()
{
  if (m_transfer_status)
  {
    throw std::runtime_error("only async request can be made to AddonManager at a time");
  }
  else
  {
    m_transfer_status = m_downloader.request_download(m_repository_url, "/addons/repository.nfo");

    m_transfer_status->then(
      [this](bool success)
      {
        m_transfer_status = {};

        if (success)
        {
          m_repository_addons = parse_addon_infos("/addons/repository.nfo");
          m_has_been_updated = true;
        }
      });

    return m_transfer_status;
  }
}

void
AddonManager::check_online()
{
  m_downloader.download(m_repository_url, "/addons/repository.nfo");
  m_repository_addons = parse_addon_infos("/addons/repository.nfo");
  m_has_been_updated = true;
}

TransferStatusPtr
AddonManager::request_install_addon(const AddonId& addon_id)
{
  if (m_transfer_status)
  {
    throw std::runtime_error("only one addon install request allowed at a time");
  }
  else
  {
    { // remove addon if it already exists
      auto it = std::find_if(m_installed_addons.begin(), m_installed_addons.end(),
                             [&addon_id](const std::unique_ptr<Addon>& addon)
                             {
                               return addon->get_id() == addon_id;
                             });
      if (it != m_installed_addons.end())
      {
        log_debug << "reinstalling addon " << addon_id << std::endl;
        if ((*it)->is_enabled())
        {
          disable_addon((*it)->get_id());
        }
        m_installed_addons.erase(it);
      }
      else
      {
        log_debug << "installing addon " << addon_id << std::endl;
      }
    }

    Addon& addon = get_repository_addon(addon_id);

    std::string install_filename = FileSystem::join(m_addon_directory, addon.get_filename());

    m_transfer_status = m_downloader.request_download(addon.get_url(), install_filename);

    m_transfer_status->then(
      [this, install_filename, addon_id](bool success)
      {
        m_transfer_status = {};

        if (success)
        {
          // complete the addon install
          Addon& repository_addon = get_repository_addon(addon_id);

          MD5 md5 = md5_from_file(install_filename);
          if (repository_addon.get_md5() != md5.hex_digest())
          {
            if (PHYSFS_delete(install_filename.c_str()) == 0)
            {
              log_warning << "PHYSFS_delete failed: " << PHYSFS_getLastError() << std::endl;
            }

            throw std::runtime_error("Downloading Add-on failed: MD5 checksums differ");
          }
          else
          {
            const char* realdir = PHYSFS_getRealDir(install_filename.c_str());
            if (!realdir)
            {
              throw std::runtime_error("PHYSFS_getRealDir failed: " + install_filename);
            }
            else
            {
              add_installed_archive(install_filename, md5.hex_digest());
            }
          }
        }
      });

    return m_transfer_status;
  }
}

void
AddonManager::install_addon(const AddonId& addon_id)
{
  { // remove addon if it already exists
    auto it = std::find_if(m_installed_addons.begin(), m_installed_addons.end(),
                           [&addon_id](const std::unique_ptr<Addon>& addon)
                           {
                             return addon->get_id() == addon_id;
                           });
    if (it != m_installed_addons.end())
    {
      log_debug << "reinstalling addon " << addon_id << std::endl;
      if ((*it)->is_enabled())
      {
        disable_addon((*it)->get_id());
      }
      m_installed_addons.erase(it);
    }
    else
    {
      log_debug << "installing addon " << addon_id << std::endl;
    }
  }

  Addon& repository_addon = get_repository_addon(addon_id);

  std::string install_filename = FileSystem::join(m_addon_directory, repository_addon.get_filename());

  m_downloader.download(repository_addon.get_url(), install_filename);

  MD5 md5 = md5_from_file(install_filename);
  if (repository_addon.get_md5() != md5.hex_digest())
  {
    if (PHYSFS_delete(install_filename.c_str()) == 0)
    {
      log_warning << "PHYSFS_delete failed: " << PHYSFS_getLastError() << std::endl;
    }

    throw std::runtime_error("Downloading Add-on failed: MD5 checksums differ");
  }
  else
  {
    const char* realdir = PHYSFS_getRealDir(install_filename.c_str());
    if (!realdir)
    {
      throw std::runtime_error("PHYSFS_getRealDir failed: " + install_filename);
    }
    else
    {
      add_installed_archive(install_filename, md5.hex_digest());
    }
  }
}

void
AddonManager::uninstall_addon(const AddonId& addon_id)
{
  log_debug << "uninstalling addon " << addon_id << std::endl;
  Addon& addon = get_installed_addon(addon_id);
  if (addon.is_enabled())
  {
    disable_addon(addon_id);
  }
  log_debug << "deleting file \"" << addon.get_install_filename() << "\"" << std::endl;
  PHYSFS_delete(addon.get_install_filename().c_str());
  m_installed_addons.erase(std::remove_if(m_installed_addons.begin(), m_installed_addons.end(),
                                          [&addon](const std::unique_ptr<Addon>& rhs)
                                          {
                                            return addon.get_id() == rhs->get_id();
                                          }),
                           m_installed_addons.end());
}

void
AddonManager::enable_addon(const AddonId& addon_id)
{
  log_debug << "enabling addon " << addon_id << std::endl;
  Addon& addon = get_installed_addon(addon_id);
  if (addon.is_enabled())
  {
    log_warning << "Tried enabling already enabled Add-on" << std::endl;
  }
  else
  {
    log_debug << "Adding archive \"" << addon.get_install_filename() << "\" to search path" << std::endl;
    //int PHYSFS_mount(addon.installed_install_filename.c_str(), "addons/", 0)
    if (PHYSFS_addToSearchPath(addon.get_install_filename().c_str(), 0) == 0)
    {
      log_warning << "Could not add " << addon.get_install_filename() << " to search path: "
                  << PHYSFS_getLastError() << std::endl;
    }
    else
    {
      addon.set_enabled(true);
    }
  }
}

void
AddonManager::disable_addon(const AddonId& addon_id)
{
  log_debug << "disabling addon " << addon_id << std::endl;
  Addon& addon = get_installed_addon(addon_id);
  if (!addon.is_enabled())
  {
    log_warning << "Tried disabling already disabled Add-On" << std::endl;
  }
  else
  {
    log_debug << "Removing archive \"" << addon.get_install_filename() << "\" from search path" << std::endl;
    if (PHYSFS_removeFromSearchPath(addon.get_install_filename().c_str()) == 0)
    {
      log_warning << "Could not remove " << addon.get_install_filename() << " from search path: "
                  << PHYSFS_getLastError() << std::endl;
    }
    else
    {
      addon.set_enabled(false);
    }
  }
}

std::vector<std::string>
AddonManager::scan_for_archives() const
{
  std::vector<std::string> archives;

  // Search for archives and add them to the search path
  std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
    rc(PHYSFS_enumerateFiles(m_addon_directory.c_str()),
       PHYSFS_freeList);
  for(char** i = rc.get(); *i != 0; ++i)
  {
    if (has_suffix(*i, ".zip"))
    {
      std::string archive = FileSystem::join(m_addon_directory, *i);
      if (PHYSFS_exists(archive.c_str()))
      {
        archives.push_back(archive);
      }
    }
  }

  return archives;
}

std::string
AddonManager::scan_for_info(const std::string& archive_os_path) const
{
  std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
    rc2(PHYSFS_enumerateFiles("/"),
        PHYSFS_freeList);
  for(char** j = rc2.get(); *j != 0; ++j)
  {
    if (has_suffix(*j, ".nfo"))
    {
      std::string nfo_filename = FileSystem::join("/", *j);

      // make sure it's in the current archive_os_path
      const char* realdir = PHYSFS_getRealDir(nfo_filename.c_str());
      if (!realdir)
      {
        log_warning << "PHYSFS_getRealDir() failed for " << nfo_filename << ": " << PHYSFS_getLastError() << std::endl;
      }
      else
      {
        if (realdir == archive_os_path)
        {
          return nfo_filename;
        }
      }
    }
  }

  return std::string();
}

void
AddonManager::add_installed_archive(const std::string& archive, const std::string& md5)
{
  const char* realdir = PHYSFS_getRealDir(archive.c_str());
  if (!realdir)
  {
    log_warning << "PHYSFS_getRealDir() failed for " << archive << ": "
                << PHYSFS_getLastError() << std::endl;
  }
  else
  {
    std::string os_path = FileSystem::join(realdir, archive);

    PHYSFS_addToSearchPath(os_path.c_str(), 0);

    std::string nfo_filename = scan_for_info(os_path);

    if (nfo_filename.empty())
    {
      log_warning << "Couldn't find .nfo file for " << os_path << std::endl;
    }
    else
    {
      try
      {
        std::unique_ptr<Addon> addon = Addon::parse(nfo_filename);
        addon->set_install_filename(os_path, md5);
        m_installed_addons.push_back(std::move(addon));
      }
      catch (const std::runtime_error& e)
      {
        log_warning << "Could not load add-on info for " << archive << ": " << e.what() << std::endl;
      }
    }

    PHYSFS_removeFromSearchPath(os_path.c_str());
  }
}

void
AddonManager::add_installed_addons()
{
  auto archives = scan_for_archives();

  for(auto archive : archives)
  {
    MD5 md5 = md5_from_file(archive);
    add_installed_archive(archive, md5.hex_digest());
  }
}

AddonManager::AddonList
AddonManager::parse_addon_infos(const std::string& filename) const
{
  AddonList m_addons;

  try
  {
    lisp::Parser parser;
    const lisp::Lisp* root = parser.parse(filename);
    const lisp::Lisp* addons_lisp = root->get_lisp("supertux-addons");
    if(!addons_lisp)
    {
      throw std::runtime_error("Downloaded file is not an Add-on list");
    }
    else
    {
      lisp::ListIterator iter(addons_lisp);
      while(iter.next())
      {
        const std::string& token = iter.item();
        if(token != "supertux-addoninfo")
        {
          log_warning << "Unknown token '" << token << "' in Add-on list" << std::endl;
        }
        else
        {
          std::unique_ptr<Addon> addon = Addon::parse(*iter.lisp());
          m_addons.push_back(std::move(addon));
        }
      }

      return m_addons;
    }
  }
  catch(const std::exception& e)
  {
    std::stringstream msg;
    msg << "Problem when reading Add-on list: " << e.what();
    throw std::runtime_error(msg.str());
  }

  return m_addons;
}

void
AddonManager::update()
{
  m_downloader.update();
}

/* EOF */
