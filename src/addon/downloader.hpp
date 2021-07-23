//  SuperTux
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

#ifndef HEADER_SUPERTUX_ADDON_DOWNLOADER_HPP
#define HEADER_SUPERTUX_ADDON_DOWNLOADER_HPP

#include <curl/curl.h>
#include <curl/easy.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

typedef int TransferId;
class Downloader;

class TransferStatus final
{
public:
  Downloader& m_downloader;
  TransferId id;
  std::vector<std::function<void (bool)> > callbacks;

  int dltotal;
  int dlnow;
  int ultotal;
  int ulnow;

  std::string error_msg;

  TransferStatus(Downloader& downloader, TransferId id_) :
    m_downloader(downloader),
    id(id_),
    callbacks(),
    dltotal(0),
    dlnow(0),
    ultotal(0),
    ulnow(0),
    error_msg()
  {}

  void abort();
  void update();

  void then(const std::function<void (bool)>& callback)
  {
    callbacks.push_back(callback);
  }
};

using TransferStatusPtr = std::shared_ptr<TransferStatus>;

class Transfer;

class Downloader final
{
private:
  CURLM* m_multi_handle;
  std::vector<std::unique_ptr<Transfer> > m_transfers;
  int m_next_transfer_id;

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

  void update();

  TransferStatusPtr request_download(const std::string& url, const std::string& filename);
  void abort(TransferId id);

private:
  Downloader(const Downloader&) = delete;
  Downloader& operator=(const Downloader&) = delete;
};

#endif

/* EOF */
