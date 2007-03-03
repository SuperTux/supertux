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

#include <stdexcept>
#include <list>
#include <physfs.h>
#include <sys/stat.h>
#include <stdio.h>
#include "addon_manager.hpp"
#include "config.h"
#include "log.hpp"

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
AddonManager::get_addons() const
{
  std::vector<Addon> addons;

  // first step: search for installed addons

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

    // extract nice title
    static const char* dirSep = PHYSFS_getDirSeparator();
    std::string::size_type n = fileName.rfind(dirSep) + 1;
    if (n == std::string::npos) n = 0;
    std::string title = fileName.substr(n, fileName.length() - n - archiveExt.length());

    Addon addon;
    addon.title = title;
    addon.fname = fileName;
    addon.isInstalled = true;

    addons.push_back(addon);
  }

#ifdef HAVE_LIBCURL
  // second step: search for available addons

  // FIXME: This URL is just for testing!
  const char* baseUrl = "http://www.deltadevelopment.de/users/christoph/supertux/addons/";
  std::string html = "";

  CURL *curl_handle;
  curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_URL, baseUrl);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "SuperTux/" PACKAGE_VERSION " libcURL");
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, my_curl_string_append);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &html);
  curl_easy_perform(curl_handle);
  curl_easy_cleanup(curl_handle);

  //std::string html = "Blubb<a href=\"http://www.deltadevelopment.de/users/christoph/supertux/addons/coconut_fortress.zip\">Coconut Fortress</a>\nFoobar<a href=\"http://www.deltadevelopment.de/users/christoph/supertux/addons/in_the_spring.zip\">Another</a>Baz";
  static const std::string startToken = "href=\"";
  static const std::string endToken = "\"";

  // extract urls: for each startToken found...
  std::string::size_type n = 0;
  while ((n = html.find(startToken)) != std::string::npos) {

    // strip everything up to and including token
    html.erase(0, n + startToken.length());

    // find end token
    std::string::size_type n2 = html.find(endToken);
    if (n2 == std::string::npos) break;

    // extract url: it's the string inbetween
    std::string url = html.substr(0, n2);

    // strip everything up to and including endToken
    html.erase(0, n2 + endToken.length());

    // make absolute url
    url = std::string(baseUrl) + url;

    // make sure url looks like it points to an archive
    static const std::string archiveExt = ".zip";
    if (url.compare(url.length()-archiveExt.length(), archiveExt.length(), archiveExt) != 0) continue;

    // extract nice title
    std::string::size_type n = url.rfind('/') + 1;
    if (n == std::string::npos) n = 0;
    std::string title = url.substr(n, url.length() - n - archiveExt.length());

    // construct file name
    std::string fname = url.substr(n);

    // make sure it does not contain weird characters
    if (fname.find_first_not_of("match.quiz-proxy_gwenblvdjfks0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) continue;

    Addon addon;
    addon.title = title;
    addon.fname = fname;
    addon.url = url;
    addon.isInstalled = false;

    addons.push_back(addon);
  }
#endif

  return addons;
}


void
AddonManager::install(const Addon& addon)
{

#ifdef HAVE_LIBCURL

  char* url = (char*)malloc(addon.url.length() + 1);
  strncpy(url, addon.url.c_str(), addon.url.length() + 1);

  PHYSFS_file* f = PHYSFS_openWrite(addon.fname.c_str());

  log_debug << "Downloading \"" << url << "\"" << std::endl;

  CURL *curl_handle;
  curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "SuperTux/" PACKAGE_VERSION " libcURL");
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, my_curl_physfs_write);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, f);
  curl_easy_perform(curl_handle);
  curl_easy_cleanup(curl_handle);

  PHYSFS_close(f);

  free(url);

  static const std::string writeDir = PHYSFS_getWriteDir();
  static const std::string dirSep = PHYSFS_getDirSeparator();
  std::string fullFilename = writeDir + dirSep + addon.fname;
  log_debug << "Finished downloading \"" << fullFilename << "\"" << std::endl;
  PHYSFS_addToSearchPath(fullFilename.c_str(), 1);
#else
  (void) addon;
#endif

}

void
AddonManager::remove(const Addon& addon)
{
  PHYSFS_removeFromSearchPath(addon.fname.c_str());
  PHYSFS_delete(addon.fname.c_str());
}
