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

#include <algorithm>
#include <memory>
#include <physfs.h>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

#ifdef HAVE_LIBCURL
#  include <curl/curl.h>
#  include <curl/easy.h>
#  include <curl/types.h>
#endif

#include "addon/addon.hpp"
#include "lisp/list_iterator.hpp"
#include "lisp/parser.hpp"
#include "util/reader.hpp"
#include "util/writer.hpp"
#include "util/log.hpp"

#ifdef HAVE_LIBCURL
namespace {

size_t my_curl_string_append(void *ptr, size_t size, size_t nmemb, void *string_ptr)
{
  std::string& s = *static_cast<std::string*>(string_ptr);
  std::string buf(static_cast<char*>(ptr), size * nmemb);
  s += buf;
  log_debug << "read " << size * nmemb << " bytes of data..." << std::endl;
  return size * nmemb;
}

size_t my_curl_physfs_write(void *ptr, size_t size, size_t nmemb, void *f_p)
{
  PHYSFS_file* f = static_cast<PHYSFS_file*>(f_p);
  PHYSFS_sint64 written = PHYSFS_write(f, ptr, size, nmemb);
  log_debug << "read " << size * nmemb << " bytes of data..." << std::endl;
  return size * written;
}

}
#endif

AddonManager&
AddonManager::get_instance()
{
  static AddonManager instance;
  return instance;
}

AddonManager::AddonManager() :
  addons(),
  ignored_addon_filenames()
{
#ifdef HAVE_LIBCURL
  curl_global_init(CURL_GLOBAL_ALL);
#endif
}

AddonManager::~AddonManager()
{
#ifdef HAVE_LIBCURL
  curl_global_cleanup();
#endif

  for (std::vector<Addon*>::iterator i = addons.begin(); i != addons.end(); i++) delete *i;
}

std::vector<Addon*>
AddonManager::get_addons()
{
  /*
    for (std::vector<Addon>::iterator it = installed_addons.begin(); it != installed_addons.end(); ++it) {
    Addon& addon = *it;
    if (addon.md5 == "") addon.md5 = calculate_md5(addon);
    }
  */
  return addons;
}

void
AddonManager::check_online()
{
#ifdef HAVE_LIBCURL
  char error_buffer[CURL_ERROR_SIZE+1];

  const char* baseUrl = "http://supertux.berlios.de/addons/index.nfo";
  std::string addoninfos = "";

  CURL *curl_handle;
  curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_URL, baseUrl);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "SuperTux/" PACKAGE_VERSION " libcURL");
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, my_curl_string_append);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &addoninfos);
  curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, error_buffer);
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);
  curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
  CURLcode result = curl_easy_perform(curl_handle);
  curl_easy_cleanup(curl_handle);

  if (result != CURLE_OK) {
    std::string why = error_buffer[0] ? error_buffer : "unhandled error";
    throw std::runtime_error("Downloading Add-on list failed: " + why);
  }

  try {
    lisp::Parser parser;
    std::stringstream addoninfos_stream(addoninfos);
    const lisp::Lisp* root = parser.parse(addoninfos_stream, "supertux-addons");

    const lisp::Lisp* addons_lisp = root->get_lisp("supertux-addons");
    if(!addons_lisp) throw std::runtime_error("Downloaded file is not an Add-on list");

    lisp::ListIterator iter(addons_lisp);
    while(iter.next()) 
    {
      const std::string& token = iter.item();
      if(token != "supertux-addoninfo") 
      {
        log_warning << "Unknown token '" << token << "' in Add-on list" << std::endl;
        continue;
      }
      std::auto_ptr<Addon> addon(new Addon());
      addon->parse(*(iter.lisp()));
      addon->installed = false;
      addon->loaded = false;

      // make sure the list of known Add-ons does not already contain this one 
      bool exists = false;
      for (std::vector<Addon*>::const_iterator i = addons.begin(); i != addons.end(); i++) {
        if (**i == *addon) {
          exists = true; 
          break; 
        }
      }

      if (exists) 
      {
        // do nothing
      }
      else if (addon->suggested_filename.find_first_not_of("match.quiz-proxy_gwenblvdjfks0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) 
      {
        // make sure the Add-on's file name does not contain weird characters
        log_warning << "Add-on \"" << addon->title << "\" contains unsafe file name. Skipping." << std::endl;
      }
      else
      {
        addons.push_back(addon.release());
      }
    }
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Problem when reading Add-on list: " << e.what();
    throw std::runtime_error(msg.str());
  }

#endif
}

void
AddonManager::install(Addon* addon)
{
#ifdef HAVE_LIBCURL

  if (addon->installed) throw std::runtime_error("Tried installing installed Add-on");

  // make sure the Add-on's file name does not contain weird characters
  if (addon->suggested_filename.find_first_not_of("match.quiz-proxy_gwenblvdjfks0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) {
    throw std::runtime_error("Add-on has unsafe file name (\""+addon->suggested_filename+"\")");
  }

  std::string fileName = addon->suggested_filename;

  // make sure its file doesn't already exist
  if (PHYSFS_exists(fileName.c_str())) {
    fileName = addon->stored_md5 + "_" + addon->suggested_filename;
    if (PHYSFS_exists(fileName.c_str())) {
      throw std::runtime_error("Add-on of suggested filename already exists (\""+addon->suggested_filename+"\", \""+fileName+"\")");
    }
  }

  char error_buffer[CURL_ERROR_SIZE+1];

  char* url = (char*)malloc(addon->http_url.length() + 1);
  strncpy(url, addon->http_url.c_str(), addon->http_url.length() + 1);

  PHYSFS_file* f = PHYSFS_openWrite(fileName.c_str());

  log_debug << "Downloading \"" << url << "\"" << std::endl;

  CURL *curl_handle;
  curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "SuperTux/" PACKAGE_VERSION " libcURL");
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, my_curl_physfs_write);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, f);
  curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, error_buffer);
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);
  curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
  CURLcode result = curl_easy_perform(curl_handle);
  curl_easy_cleanup(curl_handle);

  PHYSFS_close(f);

  free(url);

  if (result != CURLE_OK) {
    PHYSFS_delete(fileName.c_str());
    std::string why = error_buffer[0] ? error_buffer : "unhandled error";
    throw std::runtime_error("Downloading Add-on failed: " + why);
  }

  addon->installed = true;
  addon->installed_physfs_filename = fileName;
  static const std::string writeDir = PHYSFS_getWriteDir();
  static const std::string dirSep = PHYSFS_getDirSeparator();
  addon->installed_absolute_filename = writeDir + dirSep + fileName;
  addon->loaded = false;

  if (addon->get_md5() != addon->stored_md5) {
    addon->installed = false;
    PHYSFS_delete(fileName.c_str());
    std::string why = "MD5 checksums differ"; 
    throw std::runtime_error("Downloading Add-on failed: " + why);
  }

  log_debug << "Finished downloading \"" << addon->installed_absolute_filename << "\". Enabling Add-on." << std::endl;

  enable(addon);

#else
  (void) addon;
#endif

}

void
AddonManager::remove(Addon* addon)
{
  if (!addon->installed) throw std::runtime_error("Tried removing non-installed Add-on");

  //FIXME: more checks

  // make sure the Add-on's file name does not contain weird characters
  if (addon->installed_physfs_filename.find_first_not_of("match.quiz-proxy_gwenblvdjfks0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) {
    throw std::runtime_error("Add-on has unsafe file name (\""+addon->installed_physfs_filename+"\")");
  }

  unload(addon);

  log_debug << "deleting file \"" << addon->installed_absolute_filename << "\"" << std::endl;
  PHYSFS_delete(addon->installed_absolute_filename.c_str());
  addon->installed = false;

  // FIXME: As we don't know anything more about it (e.g. where to get it), remove it from list of known Add-ons
}

void
AddonManager::disable(Addon* addon)
{
  unload(addon);

  std::string fileName = addon->installed_physfs_filename;
  if (std::find(ignored_addon_filenames.begin(), ignored_addon_filenames.end(), fileName) == ignored_addon_filenames.end()) {
    ignored_addon_filenames.push_back(fileName);
  }
}

void
AddonManager::enable(Addon* addon)
{
  load(addon);

  std::string fileName = addon->installed_physfs_filename;
  std::vector<std::string>::iterator i = std::find(ignored_addon_filenames.begin(), ignored_addon_filenames.end(), fileName);
  if (i != ignored_addon_filenames.end()) {
    ignored_addon_filenames.erase(i);
  }
}

void
AddonManager::unload(Addon* addon)
{
  if (!addon->installed) throw std::runtime_error("Tried unloading non-installed Add-on");
  if (!addon->loaded) return;

  log_debug << "Removing archive \"" << addon->installed_absolute_filename << "\" from search path" << std::endl;
  if (PHYSFS_removeFromSearchPath(addon->installed_absolute_filename.c_str()) == 0) {
    log_warning << "Could not remove " << addon->installed_absolute_filename << " from search path. Ignoring." << std::endl;
    return;
  }

  addon->loaded = false;
}

void
AddonManager::load(Addon* addon)
{
  if (!addon->installed) throw std::runtime_error("Tried loading non-installed Add-on");
  if (addon->loaded) return;

  log_debug << "Adding archive \"" << addon->installed_absolute_filename << "\" to search path" << std::endl;
  if (PHYSFS_addToSearchPath(addon->installed_absolute_filename.c_str(), 0) == 0) {
    log_warning << "Could not add " << addon->installed_absolute_filename << " to search path. Ignoring." << std::endl;
    return;
  }

  addon->loaded = true;
}

void
AddonManager::load_addons()
{
  // unload all Addons and forget about them
  for (std::vector<Addon*>::iterator i = addons.begin(); i != addons.end(); i++) {
    if ((*i)->installed && (*i)->loaded) unload(*i);
    delete *i;
  }
  addons.clear();

  // Search for archives and add them to the search path
  char** rc = PHYSFS_enumerateFiles("/");

  for(char** i = rc; *i != 0; ++i) {

    // get filename of potential archive
    std::string fileName = *i;

    const std::string archiveDir = PHYSFS_getRealDir(fileName.c_str());
    static const std::string dirSep = PHYSFS_getDirSeparator();
    std::string fullFilename = archiveDir + dirSep + fileName;

    /*
    // make sure it's in the writeDir
    static const std::string writeDir = PHYSFS_getWriteDir();
    if (fileName.compare(0, writeDir.length(), writeDir) != 0) continue;
    */

    // make sure it looks like an archive
    static const std::string archiveExt = ".zip";
    if (fullFilename.compare(fullFilename.length()-archiveExt.length(), archiveExt.length(), archiveExt) != 0) continue;

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
    char** rc2 = PHYSFS_enumerateFiles("/");
    for(char** i = rc2; *i != 0; ++i) {

      // get filename of potential infoFile
      std::string potentialInfoFileName = *i;

      // make sure it looks like an infoFile
      static const std::string infoExt = ".nfo";
      if (potentialInfoFileName.length() <= infoExt.length())
        continue;

      if (potentialInfoFileName.compare(potentialInfoFileName.length()-infoExt.length(), infoExt.length(), infoExt) != 0)
        continue;

      // make sure it's in the current archive
      std::string infoFileDir = PHYSFS_getRealDir(potentialInfoFileName.c_str());
      if (infoFileDir != fullFilename) continue;

      // found infoFileName
      infoFileName = potentialInfoFileName;
      break;
    }
    PHYSFS_freeList(rc2);

    // if we have an infoFile, it's an Addon
    if (infoFileName != "") {
      try {
        Addon* addon = new Addon();
        addon->parse(infoFileName);
        addon->installed = true;
        addon->installed_physfs_filename = fileName;
        addon->installed_absolute_filename = fullFilename;
        addon->loaded = true;
        addons.push_back(addon);

        // check if the Addon is disabled 
        if (std::find(ignored_addon_filenames.begin(), ignored_addon_filenames.end(), fileName) != ignored_addon_filenames.end()) {
          unload(addon);
        }

      } catch (const std::runtime_error& e) {
        log_warning << "Could not load add-on info for " << fullFilename << ", loading as unmanaged:" << e.what() << std::endl;
      }
    }

  }

  PHYSFS_freeList(rc);
}

void
AddonManager::read(const Reader& lisp)
{
  lisp.get("disabled-addons", ignored_addon_filenames); 
}

void
AddonManager::write(lisp::Writer& writer)
{
  writer.write("disabled-addons", ignored_addon_filenames); 
}

/* EOF */
