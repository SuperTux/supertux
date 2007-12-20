//  $Id$
//
//  SuperTux - Add-on Manager
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//

#include <sstream>
#include <stdexcept>
#include <list>
#include <physfs.h>
#include <sys/stat.h>
#include <stdio.h>
#include "addon_manager.hpp"
#include "config.h"
#include "log.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "lisp/list_iterator.hpp"

#ifdef HAVE_LIBCURL
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#endif

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

AddonManager::AddonManager()
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
}

std::vector<Addon>
AddonManager::get_installed_addons() const
{
  std::vector<Addon> addons;

  // iterate over complete search path (i.e. directories and archives)
  char **i = PHYSFS_getSearchPath();
  if (!i) throw std::runtime_error("Could not query physfs search path");
  for (; *i != NULL; i++) {

    // get filename of potential archive
    std::string fileName = *i;

    // make sure it's in the writeDir
    static const std::string writeDir = PHYSFS_getWriteDir();
    if (fileName.compare(0, writeDir.length(), writeDir) != 0) continue;

    // make sure it looks like an archive
    static const std::string archiveExt = ".zip";
    if (fileName.compare(fileName.length()-archiveExt.length(), archiveExt.length(), archiveExt) != 0) continue;

    // make sure it exists
    struct stat stats;
    if (stat(fileName.c_str(), &stats) != 0) continue;

    // make sure it's an actual file
    if (!S_ISREG(stats.st_mode)) continue;

    Addon addon;

    // extract nice title as fallback for when the Add-on has no addoninfo file
    static const char* dirSep = PHYSFS_getDirSeparator();
    std::string::size_type n = fileName.rfind(dirSep) + 1;
    if (n == std::string::npos) n = 0;
    addon.title = fileName.substr(n, fileName.length() - n - archiveExt.length());
    std::string shortFileName = fileName.substr(n, fileName.length() - n);
    addon.file = shortFileName;
   
    // read an accompaining .nfo file, if it exists
    static const std::string infoExt = ".nfo";
    std::string infoFileName = fileName.substr(n, fileName.length() - n - archiveExt.length()) + infoExt;
    if (PHYSFS_exists(infoFileName.c_str())) {
      addon.parse(infoFileName);
      if (addon.file != shortFileName) {
        log_warning << "Add-on \"" << addon.title << "\", contained in file \"" << shortFileName << "\" is accompained by an addoninfo file that specifies \"" << addon.file << "\" as the Add-on's file name. Skipping." << std::endl;
      }
    }

    // make sure the Add-on's file name does not contain weird characters
    if (addon.file.find_first_not_of("match.quiz-proxy_gwenblvdjfks0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) {
      log_warning << "Add-on \"" << addon.title << "\" contains unsafe file name. Skipping." << std::endl;
      continue;
    }

    addon.isInstalled = true;
    addons.push_back(addon);
  }

  return addons;
}

std::vector<Addon>
AddonManager::get_available_addons() const
{
  std::vector<Addon> addons;

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
    while(iter.next()) {
      const std::string& token = iter.item();
      if(token == "supertux-addoninfo") {
        Addon addon;
	addon.parse(*(iter.lisp()));

        // make sure the Add-on's file name does not contain weird characters
        if (addon.file.find_first_not_of("match.quiz-proxy_gwenblvdjfks0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) {
          log_warning << "Add-on \"" << addon.title << "\" contains unsafe file name. Skipping." << std::endl;
          continue;
        }

        addon.isInstalled = false;
        addons.push_back(addon);
      } else {
        log_warning << "Unknown token '" << token << "' in Add-on list" << std::endl;
      }
    }
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Problem when reading Add-on list: " << e.what();
    throw std::runtime_error(msg.str());
  }

#endif

  return addons;
}


void
AddonManager::install(const Addon& addon)
{
  // make sure the Add-on's file name does not contain weird characters
  if (addon.file.find_first_not_of("match.quiz-proxy_gwenblvdjfks0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) {
    throw std::runtime_error("Add-on has unsafe file name (\""+addon.file+"\")");
  }

#ifdef HAVE_LIBCURL

  char error_buffer[CURL_ERROR_SIZE+1];

  char* url = (char*)malloc(addon.http_url.length() + 1);
  strncpy(url, addon.http_url.c_str(), addon.http_url.length() + 1);

  PHYSFS_file* f = PHYSFS_openWrite(addon.file.c_str());

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
    PHYSFS_delete(addon.file.c_str());
    std::string why = error_buffer[0] ? error_buffer : "unhandled error";
    throw std::runtime_error("Downloading Add-on failed: " + why);
  }

  // write an accompaining .nfo file
  static const std::string archiveExt = ".zip";
  static const std::string infoExt = ".nfo";
  std::string infoFileName = addon.file.substr(0, addon.file.length()-archiveExt.length()) + infoExt;
  addon.write(infoFileName);

  static const std::string writeDir = PHYSFS_getWriteDir();
  static const std::string dirSep = PHYSFS_getDirSeparator();
  std::string fullFilename = writeDir + dirSep + addon.file;
  log_debug << "Finished downloading \"" << fullFilename << "\"" << std::endl;
  PHYSFS_addToSearchPath(fullFilename.c_str(), 1);
#else
  (void) addon;
#endif

}

void
AddonManager::remove(const Addon& addon)
{
  // make sure the Add-on's file name does not contain weird characters
  if (addon.file.find_first_not_of("match.quiz-proxy_gwenblvdjfks0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) {
    throw std::runtime_error("Add-on has unsafe file name (\""+addon.file+"\")");
  }

  log_debug << "deleting file \"" << addon.file << "\"" << std::endl;
  PHYSFS_removeFromSearchPath(addon.file.c_str());
  PHYSFS_delete(addon.file.c_str());

  // remove an accompaining .nfo file
  static const std::string archiveExt = ".zip";
  static const std::string infoExt = ".nfo";
  std::string infoFileName = addon.file.substr(0, addon.file.length()-archiveExt.length()) + infoExt;
  if (PHYSFS_exists(infoFileName.c_str())) {
    log_debug << "deleting file \"" << infoFileName << "\"" << std::endl;
    PHYSFS_delete(infoFileName.c_str());
  }
}

