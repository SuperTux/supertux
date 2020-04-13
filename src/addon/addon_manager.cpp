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

#include <physfs.h>

#include "addon/addon.hpp"
#include "addon/md5.hpp"
#include "physfs/util.hpp"
#include "supertux/globals.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/string_util.hpp"

namespace {

static const char* ADDON_INFO_PATH = "/addons/repository.nfo";

MD5 md5_from_file(const std::string& filename)
{
  // TODO: this does not work as expected for some files -- IFileStream seems to not always behave like an ifstream.
  //IFileStream ifs(installed_physfs_filename);
  //std::string md5 = MD5(ifs).hex_digest();

  MD5 md5;

  auto file = PHYSFS_openRead(filename.c_str());
  if (!file)
  {
    std::ostringstream out;
    out << "PHYSFS_openRead() failed: " << PHYSFS_getLastErrorCode();
    throw std::runtime_error(out.str());
  }
  else
  {
    while (true)
    {
      unsigned char buffer[1024];
      PHYSFS_sint64 len = PHYSFS_readBytes(file, buffer, sizeof(buffer));
      if (len <= 0) break;
      md5.update(buffer, static_cast<unsigned int>(len));
    }
    PHYSFS_close(file);

    return md5;
  }
}

MD5 md5_from_archive(const std::string& filename)
{
  if (physfsutil::is_directory(filename)) {
    return MD5();
  } else {
    return md5_from_file(filename);
  }
}

static Addon& get_addon(const AddonManager::AddonList& list, const AddonId& id,
                        bool installed)
{
  auto it = std::find_if(list.begin(), list.end(),
                         [&id](const std::unique_ptr<Addon>& addon)
                         {
                           return addon->get_id() == id;
                         });

  if (it != list.end())
  {
    return **it;
  }
  else
  {
    std::string type = installed ? "installed" : "repository";
    throw std::runtime_error("Couldn't find " + type + " addon with id: " + id);
  }
}

static std::vector<AddonId> get_addons(const AddonManager::AddonList& list)
{
  std::vector<AddonId> results;
  results.reserve(list.size());
  std::transform(list.begin(), list.end(),
                 std::back_inserter(results),
                 [](const std::unique_ptr<Addon>& addon)
                 {
                   return addon->get_id();
                 });
  return results;
}

static PHYSFS_EnumerateCallbackResult add_to_dictionary_path(void *data, const char *origdir, const char *fname)
{
    std::string full_path = FileSystem::join(origdir, fname);
    if (physfsutil::is_directory(full_path))
    {
        log_debug << "Adding \"" << full_path << "\" to dictionary search path" << std::endl;
        // We want translations from addons to have precedence
        g_dictionary_manager->add_directory(full_path, true);
    }
    return PHYSFS_ENUM_OK;
}

static PHYSFS_EnumerateCallbackResult remove_from_dictionary_path(void *data, const char *origdir, const char *fname)
{
    std::string full_path = FileSystem::join(origdir, fname);
    if (physfsutil::is_directory(full_path))
    {
        g_dictionary_manager->remove_directory(full_path);
    }
    return PHYSFS_ENUM_OK;
}
} // namespace

AddonManager::AddonManager(const std::string& addon_directory,
                           std::vector<Config::Addon>& addon_config) :
  m_downloader(),
  m_addon_directory(addon_directory),
  m_repository_url("https://raw.githubusercontent.com/SuperTux/addons/master/index-0_6.nfo"),
  m_addon_config(addon_config),
  m_installed_addons(),
  m_repository_addons(),
  m_has_been_updated(false),
  m_transfer_status()
{
  if (!PHYSFS_mkdir(m_addon_directory.c_str()))
  {
    std::ostringstream msg;
    msg << "Couldn't create directory for addons '"
        << m_addon_directory << "': " << PHYSFS_getLastErrorCode();
    throw std::runtime_error(msg.str());
  }

  add_installed_addons();

  // FIXME: We should also restore the order here
  for (auto& addon : m_addon_config)
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

  if (PHYSFS_exists(ADDON_INFO_PATH))
  {
    try
    {
        m_repository_addons = parse_addon_infos(ADDON_INFO_PATH);
    }
    catch(const std::exception& err)
    {
        log_warning << "parsing repository.nfo failed: " << err.what() << std::endl;
    }
  }
  else
  {
    log_info << "repository.nfo doesn't exist, not loading" << std::endl;
  }

  if (!g_config->repository_url.empty() &&
      g_config->repository_url != m_repository_url)
  {
    m_repository_url = g_config->repository_url;
  }
}

AddonManager::~AddonManager()
{
  // sync enabled/disabled addons into the config for saving
  m_addon_config.clear();
  for (const auto& addon : m_installed_addons)
  {
    m_addon_config.push_back({addon->get_id(), addon->is_enabled()});
  }
}

Addon&
AddonManager::get_repository_addon(const AddonId& id) const
{
  return get_addon(m_repository_addons, id, false);
}

Addon&
AddonManager::get_installed_addon(const AddonId& id) const
{
  return get_addon(m_installed_addons, id, true);
}

std::vector<AddonId>
AddonManager::get_repository_addons() const
{
  return get_addons(m_repository_addons);
}


std::vector<AddonId>
AddonManager::get_installed_addons() const
{
  return get_addons(m_installed_addons);
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
    m_transfer_status = m_downloader.request_download(m_repository_url, ADDON_INFO_PATH);

    m_transfer_status->then(
      [this](bool success)
      {
        m_transfer_status = {};

        if (success)
        {
          m_repository_addons = parse_addon_infos(ADDON_INFO_PATH);
          m_has_been_updated = true;
        }
      });

    return m_transfer_status;
  }
}

void
AddonManager::check_online()
{
  m_downloader.download(m_repository_url, ADDON_INFO_PATH);
  m_repository_addons = parse_addon_infos(ADDON_INFO_PATH);
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

    auto& addon = get_repository_addon(addon_id);

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
              log_warning << "PHYSFS_delete failed: " << PHYSFS_getLastErrorCode() << std::endl;
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

  auto& repository_addon = get_repository_addon(addon_id);

  std::string install_filename = FileSystem::join(m_addon_directory, repository_addon.get_filename());

  m_downloader.download(repository_addon.get_url(), install_filename);

  MD5 md5 = md5_from_file(install_filename);
  if (repository_addon.get_md5() != md5.hex_digest())
  {
    if (PHYSFS_delete(install_filename.c_str()) == 0)
    {
      log_warning << "PHYSFS_delete failed: " << PHYSFS_getLastErrorCode() << std::endl;
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
  auto& addon = get_installed_addon(addon_id);
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
  auto& addon = get_installed_addon(addon_id);
  if (addon.is_enabled())
  {
    log_warning << "Tried enabling already enabled Add-on" << std::endl;
  }
  else
  {
    log_debug << "Adding archive \"" << addon.get_install_filename() << "\" to search path" << std::endl;
    //int PHYSFS_mount(addon.installed_install_filename.c_str(), "addons/", 0)

    std::string mountpoint;
    switch (addon.get_format()) {
      case Addon::ORIGINAL:
        mountpoint = "";
        break;
      default:
        mountpoint = "custom/" + addon_id;
        break;
    }

    if (PHYSFS_mount(addon.get_install_filename().c_str(), mountpoint.c_str(), 1) == 0)
    {
      log_warning << "Could not add " << addon.get_install_filename() << " to search path: "
                  << PHYSFS_getLastErrorCode() << std::endl;
    }
    else
    {
      if (addon.get_type() == Addon::LANGUAGEPACK)
      {
        PHYSFS_enumerate(addon.get_id().c_str(), add_to_dictionary_path, nullptr);
      }
      addon.set_enabled(true);
    }
  }
}

void
AddonManager::disable_addon(const AddonId& addon_id)
{
  log_debug << "disabling addon " << addon_id << std::endl;
  auto& addon = get_installed_addon(addon_id);
  if (!addon.is_enabled())
  {
    log_warning << "Tried disabling already disabled Add-On" << std::endl;
  }
  else
  {
    log_debug << "Removing archive \"" << addon.get_install_filename() << "\" from search path" << std::endl;
    if (PHYSFS_unmount(addon.get_install_filename().c_str()) == 0)
    {
      log_warning << "Could not remove " << addon.get_install_filename() << " from search path: "
                  << PHYSFS_getLastErrorCode() << std::endl;
    }
    else
    {
      if (addon.get_type() == Addon::LANGUAGEPACK)
      {
        PHYSFS_enumerate(addon.get_id().c_str(), remove_from_dictionary_path, nullptr);
      }
      addon.set_enabled(false);
    }
  }
}

bool
AddonManager::is_old_enabled_addon(const std::unique_ptr<Addon>& addon) const
{
  return addon->get_format() == Addon::ORIGINAL &&
         addon->get_type() != Addon::LANGUAGEPACK &&
         addon->is_enabled();
}

bool
AddonManager::is_old_addon_enabled() const {
  auto it = std::find_if(m_installed_addons.begin(), m_installed_addons.end(),
                         [this](const std::unique_ptr<Addon>& addon)
                         {
                           return is_old_enabled_addon(addon);
                         });

  return it != m_installed_addons.end();
}

void
AddonManager::disable_old_addons()
{
  for (auto& addon : m_installed_addons) {
    if (is_old_enabled_addon(addon)) {
      disable_addon(addon->get_id());
    }
  }
}

void
AddonManager::mount_old_addons()
{
  std::string mountpoint;
  for (auto& addon : m_installed_addons) {
    if (is_old_enabled_addon(addon)) {
      if (PHYSFS_mount(addon->get_install_filename().c_str(), mountpoint.c_str(), 1) == 0)
      {
        log_warning << "Could not add " << addon->get_install_filename() << " to search path: "
                    << PHYSFS_getLastErrorCode() << std::endl;
      }
    }
  }
}

void
AddonManager::unmount_old_addons()
{
  for (auto& addon : m_installed_addons) {
    if (is_old_enabled_addon(addon)) {
      if (PHYSFS_unmount(addon->get_install_filename().c_str()) == 0)
      {
        log_warning << "Could not remove " << addon->get_install_filename() << " from search path: "
                    << PHYSFS_getLastErrorCode() << std::endl;
      }
    }
  }
}

bool
AddonManager::is_from_old_addon(const std::string& filename) const
{
  std::string real_path = PHYSFS_getRealDir(filename.c_str());
  for (auto& addon : m_installed_addons) {
    if (is_old_enabled_addon(addon) &&
        addon->get_install_filename() == real_path) {
      return true;
    }
  }
  return false;
}

bool
AddonManager::is_addon_installed(const std::string& id) const
{
  return std::any_of(get_installed_addons().begin(), get_installed_addons().end(),
    [id] (const auto& installed_addon) {
      return installed_addon == id;
    });
}

std::vector<std::string>
AddonManager::scan_for_archives() const
{
  std::vector<std::string> archives;

  // Search for archives and add them to the search path
  std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
    rc(PHYSFS_enumerateFiles(m_addon_directory.c_str()),
       PHYSFS_freeList);
  for (char** i = rc.get(); *i != nullptr; ++i)
  {
    const std::string fullpath = FileSystem::join(m_addon_directory, *i);
    if (physfsutil::is_directory(fullpath))
    {
      // ignore dot files (e.g. '.git/')
      if ((*i)[0] != '.') {
        archives.push_back(fullpath);
      }
    }
    else
    {
      if (StringUtil::has_suffix(StringUtil::tolower(*i), ".zip")) {
        if (PHYSFS_exists(fullpath.c_str())) {
          archives.push_back(fullpath);
        }
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
  for (char** j = rc2.get(); *j != nullptr; ++j)
  {
    if (StringUtil::has_suffix(*j, ".nfo"))
    {
      std::string nfo_filename = FileSystem::join("/", *j);

      // make sure it's in the current archive_os_path
      const char* realdir = PHYSFS_getRealDir(nfo_filename.c_str());
      if (!realdir)
      {
        log_warning << "PHYSFS_getRealDir() failed for " << nfo_filename << ": " << PHYSFS_getLastErrorCode() << std::endl;
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
                << PHYSFS_getLastErrorCode() << std::endl;
  }
  else
  {
    std::string os_path = FileSystem::join(realdir, archive);

    PHYSFS_mount(os_path.c_str(), nullptr, 1);

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

    PHYSFS_unmount(os_path.c_str());
  }
}

void
AddonManager::add_installed_addons()
{
  auto archives = scan_for_archives();

  for (const auto& archive : archives)
  {
    MD5 md5 = md5_from_archive(archive);
    add_installed_archive(archive, md5.hex_digest());
  }
}

AddonManager::AddonList
AddonManager::parse_addon_infos(const std::string& filename) const
{
  AddonList m_addons;

  try
  {
    register_translation_directory(filename);
    auto doc = ReaderDocument::from_file(filename);
    auto root = doc.get_root();
    if (root.get_name() != "supertux-addons")
    {
      throw std::runtime_error("Downloaded file is not an Add-on list");
    }
    else
    {
      auto addon_collection = root.get_collection();
      for (auto const& addon_node : addon_collection.get_objects())
      {
        if (addon_node.get_name() != "supertux-addoninfo")
        {
          log_warning << "Unknown token '" << addon_node.get_name() << "' in Add-on list" << std::endl;
        }
        else
        {
          try
          {
            std::unique_ptr<Addon> addon = Addon::parse(addon_node.get_mapping());
            m_addons.push_back(std::move(addon));
          }
          catch(const std::exception& e)
          {
            log_warning << "Problem when reading Add-on entry: " << e.what() << std::endl;
          }
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
}

void
AddonManager::update()
{
  m_downloader.update();
}

void
AddonManager::check_for_langpack_updates()
{
  const std::string& language = g_dictionary_manager->get_language().get_language();
  if (language == "en")
    return;

  try
  {
    check_online();
    try
    {
      const std::string& addon_id = "language-pack";
      log_debug << "Looking for language addon with ID " << addon_id << "..." << std::endl;
      Addon& langpack = get_repository_addon(addon_id);

      try
      {
        auto& installed_langpack = get_installed_addon(addon_id);
        if (installed_langpack.get_md5() == langpack.get_md5() ||
            installed_langpack.get_version() > langpack.get_version())
        {
          log_debug << "Language addon " << addon_id << " is already the latest version." << std::endl;
          return;
        }

        // Langpack update available. Let's install it!
        install_addon(addon_id);
        enable_addon(addon_id);
      }
      catch(const std::exception&)
      {
        log_debug << "Language addon " << addon_id << " is not installed. Installing..." << std::endl;
        install_addon(addon_id);
        enable_addon(addon_id);
      }
    }
    catch(std::exception&)
    {
      log_debug << "Language addon for current locale not found." << std::endl;
    }
  }
  catch(...)
  {
    // If anything fails here, just silently ignore.
  }
}

/* EOF */
