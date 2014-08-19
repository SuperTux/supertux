//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "addon/downloader.hpp"

#include <curl/curl.h>
#include <curl/easy.h>
#include <physfs.h>
#include <memory>
#include <stdexcept>

#include "util/log.hpp"
#include "version.h"

namespace {

size_t my_curl_string_append(void* ptr, size_t size, size_t nmemb, void* userdata)
{
  std::string& s = *static_cast<std::string*>(userdata);
  std::string buf(static_cast<char*>(ptr), size * nmemb);
  s += buf;
  log_debug << "read " << size * nmemb << " bytes of data..." << std::endl;
  return size * nmemb;
}

size_t my_curl_physfs_write(void* ptr, size_t size, size_t nmemb, void* userdata)
{
  PHYSFS_file* f = static_cast<PHYSFS_file*>(userdata);
  PHYSFS_sint64 written = PHYSFS_write(f, ptr, size, nmemb);
  log_debug << "read " << size * nmemb << " bytes of data..." << std::endl;
  return size * written;
}

} // namespace

Downloader::Downloader()
{
  curl_global_init(CURL_GLOBAL_ALL);
}

Downloader::~Downloader()
{
  curl_global_cleanup();
}

void
Downloader::download(const std::string& url,
                     size_t (*write_func)(void* ptr, size_t size, size_t nmemb, void* userdata),
                     void* userdata)
{
  char error_buffer[CURL_ERROR_SIZE+1];

  CURL* curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "SuperTux/" PACKAGE_VERSION " libcURL");
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_func);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, userdata);
  curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, error_buffer);
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);
  curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
  CURLcode result = curl_easy_perform(curl_handle);
  curl_easy_cleanup(curl_handle);

  if (result != CURLE_OK)
  {
    std::string why = error_buffer[0] ? error_buffer : "unhandled error";
    throw std::runtime_error(url + ": download failed: " + why);
  }
}

std::string
Downloader::download(const std::string& url)
{
  std::string result;
  download(url, my_curl_string_append, &result);
  return result;
}

void
Downloader::download(const std::string& url, const std::string& filename)
{
  std::unique_ptr<PHYSFS_file, int(*)(PHYSFS_File*)> fout(PHYSFS_openWrite(filename.c_str()),
                                                          PHYSFS_close);
  download(url, my_curl_physfs_write, fout.get());
}

/* EOF */
