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

#ifndef HEADER_SUPERTUX_ADDON_DOWNLOADER_HPP
#define HEADER_SUPERTUX_ADDON_DOWNLOADER_HPP

#include <string>

class Downloader
{
private:
public:
  Downloader();
  ~Downloader();

  /** Download \a url and return the result as string */
  std::string download(const std::string& url);

  /** Download \a url and store the result in \a filename */
  void download(const std::string& url, const std::string& filename);

  void download(const std::string& url,
                size_t (*write_func)(void* ptr, size_t size, size_t nmemb, void* userdata),
                void* userdata);

private:
  Downloader(const Downloader&) = delete;
  Downloader& operator=(const Downloader&) = delete;
};

#endif

/* EOF */
