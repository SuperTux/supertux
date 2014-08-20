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

#include "addon/addon_manager.hpp"

#include <config.h>
#include <version.h>
#include <iostream>

#include <algorithm>
#include <memory>
#include <physfs.h>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

#include "addon/addon.hpp"
#include "addon/addon_list.hpp"
#include "lisp/list_iterator.hpp"
#include "lisp/parser.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "util/writer.hpp"

namespace {

const char* allowed_characters = "-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";

} // namespace


AddonManager::AddonManager(const std::string& addon_directory,
                           std::vector<std::string>& ignored_addon_filenames) :
  m_downloader(),
  m_addon_directory(addon_directory),
  m_addons(),
  m_ignored_addon_filenames(ignored_addon_filenames)
{
}

AddonManager::~AddonManager()
{
}

Addon&
AddonManager::get_addon(int id)
{
  if (0 <= id && id < static_cast<int>(m_addons.size()))
  {
    return *m_addons[id];
  }
  else
  {
    throw std::runtime_error("AddonManager::get_addon(): id out of range: " + std::to_string(id));
  }
}

const std::vector<std::unique_ptr<Addon> >&
AddonManager::get_addons() const
{
  /*
    for (std::vector<Addon>::iterator it = installed_addons.begin(); it != installed_addons.end(); ++it) {
    Addon& addon = *it;
    if (addon.md5.empty()) addon.md5 = calculate_md5(addon);
    }
  */
  return m_addons;
}

bool
AddonManager::has_online_support() const
{
#ifdef HAVE_LIBCURL
  return true;
#else
  return false;
#endif
}

void
AddonManager::check_online()
{
  const char* baseUrl = "http://addons.supertux.googlecode.com/git/index-0_3_5.nfo";
  std::string addoninfos = m_downloader.download(baseUrl);

  AddonList::parse(addoninfos);
}

void
AddonManager::install(Addon& addon)
{
  if (addon.installed)
  {
    throw std::runtime_error("Tried installing installed Add-on");
  }

  // make sure the Add-on's file name does not contain weird characters
  if (addon.suggested_filename.find_first_not_of(allowed_characters) != std::string::npos)
  {
    throw std::runtime_error("Add-on has unsafe file name (\""+addon.suggested_filename+"\")");
  }

  std::string filename = FileSystem::join(m_addon_directory, addon.suggested_filename);

  // make sure its file doesn't already exist
  if (PHYSFS_exists(filename.c_str()))
  {
    filename = FileSystem::join(m_addon_directory, addon.stored_md5 + "_" + addon.suggested_filename);
    if (PHYSFS_exists(filename.c_str()))
    {
      throw std::runtime_error("Add-on of suggested filename already exists (\"" +
                               addon.suggested_filename + "\", \"" + filename + "\")");
    }
  }

  m_downloader.download(addon.http_url, filename);

  addon.installed = true;
  addon.installed_physfs_filename = filename;
  std::string writeDir = PHYSFS_getWriteDir();
  addon.installed_absolute_filename = FileSystem::join(writeDir, filename);
  addon.loaded = false;

  if (addon.get_md5() != addon.stored_md5)
  {
    addon.installed = false;
    PHYSFS_delete(filename.c_str());
    std::string why = "MD5 checksums differ";
    throw std::runtime_error("Downloading Add-on failed: " + why);
  }

  log_debug << "Finished downloading \"" << addon.installed_absolute_filename << "\". Enabling Add-on." << std::endl;

  enable(addon);
}

void
AddonManager::remove(Addon& addon)
{
  if (!addon.installed)
  {
    throw std::runtime_error("Tried removing non-installed Add-on");
  }
  else if (addon.installed_physfs_filename.find_first_not_of(allowed_characters) != std::string::npos)
  {
    // make sure the Add-on's file name does not contain weird characters
    throw std::runtime_error("Add-on has unsafe file name (\""+addon.installed_physfs_filename+"\")");
  }
  else
  {
    unload(addon);

    log_debug << "deleting file \"" << addon.installed_absolute_filename << "\"" << std::endl;
    PHYSFS_delete(addon.installed_absolute_filename.c_str());
    addon.installed = false;

    // FIXME: As we don't know anything more about it (e.g. where to get it), remove it from list of known Add-ons
  }
}

void
AddonManager::disable(Addon& addon)
{
  unload(addon);

  std::string filename = addon.installed_physfs_filename;
  if (std::find(m_ignored_addon_filenames.begin(), m_ignored_addon_filenames.end(),
                filename) == m_ignored_addon_filenames.end())
  {
    m_ignored_addon_filenames.push_back(filename);
  }
}

void
AddonManager::enable(Addon& addon)
{
  load(addon);

  std::string filename = addon.installed_physfs_filename;
  auto it = std::find(m_ignored_addon_filenames.begin(), m_ignored_addon_filenames.end(), filename);
  if (it != m_ignored_addon_filenames.end())
  {
    m_ignored_addon_filenames.erase(it);
  }
}

void
AddonManager::unload(Addon& addon)
{
  if (!addon.installed)
  {
    throw std::runtime_error("Tried unloading non-installed Add-on");
  }
  else if (!addon.loaded)
  {
    // do nothing
  }
  else
  {
    log_debug << "Removing archive \"" << addon.installed_absolute_filename << "\" from search path" << std::endl;
    if (PHYSFS_removeFromSearchPath(addon.installed_absolute_filename.c_str()) == 0) {
      log_warning << "Could not remove " << addon.installed_absolute_filename << " from search path. Ignoring." << std::endl;
      return;
    }

    addon.loaded = false;
  }
}

void
AddonManager::load(Addon& addon)
{
  if (!addon.installed)
  {
    throw std::runtime_error("Tried loading non-installed Add-on");
  }
  else if (addon.loaded)
  {
    // do nothing
  }
  else
  {
    log_debug << "Adding archive \"" << addon.installed_absolute_filename << "\" to search path" << std::endl;
    if (PHYSFS_addToSearchPath(addon.installed_absolute_filename.c_str(), 0) == 0) {
      log_warning << "Could not add " << addon.installed_absolute_filename << " to search path. Ignoring." << std::endl;
      return;
    }

    addon.loaded = true;
  }
}

void
AddonManager::load_addons()
{
  PHYSFS_mkdir(m_addon_directory.c_str());

  // unload all Addons and forget about them
  for (auto& addon : m_addons)
  {
    if (addon->installed && addon->loaded)
    {
      unload(*addon);
    }
  }
  m_addons.clear();

  // Search for archives and add them to the search path
  char** rc = PHYSFS_enumerateFiles(m_addon_directory.c_str());

  for(char** i = rc; *i != 0; ++i)
  {
    // get filename of potential archive
    std::string filename = *i;

    std::cout << m_addon_directory << " -> " << filename << std::endl;

    const std::string archiveDir = PHYSFS_getRealDir(filename.c_str());
    std::string fullFilename = FileSystem::join(archiveDir, filename);

    /*
    // make sure it's in the writeDir
    std::string writeDir = PHYSFS_getWriteDir();
    if (filename.compare(0, writeDir.length(), writeDir) != 0) continue;
    */

    // make sure it looks like an archive
    std::string archiveExt = ".zip";
    if (fullFilename.compare(fullFilename.length() - archiveExt.length(),
                             archiveExt.length(), archiveExt) != 0)
    {
      continue;
    }

    // make sure it exists
    struct stat stats;
    if (stat(fullFilename.c_str(), &stats) != 0) continue;

    // make sure it's an actual file
    if (!S_ISREG(stats.st_mode)) continue;

    log_debug << "Found archive \"" << fullFilename << "\"" << std::endl;

    // add archive to search path
    PHYSFS_addToSearchPath(fullFilename.c_str(), 0);

    // Search for infoFiles
    std::string infoFileName = "";
    char** rc2 = PHYSFS_enumerateFiles(m_addon_directory.c_str());
    for(char** j = rc2; *j != 0; ++j)
    {
      // get filename of potential infoFile
      std::string potentialInfoFileName = *j;

      // make sure it looks like an infoFile
      static const std::string infoExt = ".nfo";
      if (potentialInfoFileName.length() <= infoExt.length())
        continue;

      if (potentialInfoFileName.compare(potentialInfoFileName.length()-infoExt.length(), infoExt.length(), infoExt) != 0)
        continue;

      // make sure it's in the current archive
      std::string infoFileDir = PHYSFS_getRealDir(potentialInfoFileName.c_str());
      if (infoFileDir == fullFilename)
      {
        // found infoFileName
        infoFileName = potentialInfoFileName;
        break;
      }
    }
    PHYSFS_freeList(rc2);

    // if we have an infoFile, it's an Addon
    if (!infoFileName.empty())
    {
      try
      {
        std::unique_ptr<Addon> addon(new Addon(m_addons.size()));
        addon->parse(infoFileName);
        addon->installed = true;
        addon->installed_physfs_filename = filename;
        addon->installed_absolute_filename = fullFilename;
        addon->loaded = true;

        // check if the Addon is disabled
        if (std::find(m_ignored_addon_filenames.begin(), m_ignored_addon_filenames.end(), filename) != m_ignored_addon_filenames.end())
        {
          unload(*addon);
        }

        m_addons.push_back(std::move(addon));
      }
      catch (const std::runtime_error& e)
      {
        log_warning << "Could not load add-on info for " << fullFilename << ", loading as unmanaged:" << e.what() << std::endl;
      }
    }
  }

  PHYSFS_freeList(rc);
}

/* EOF */
