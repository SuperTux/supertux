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

#ifndef __EMSCRIPTEN__

#include "addon/downloader.hpp"

#include <algorithm>
#include <array>
#include <assert.h>
#include <memory>
#include <physfs.h>
#include <sstream>
#include <stdexcept>
#include <version.h>

#include "util/log.hpp"

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
  PHYSFS_sint64 written = PHYSFS_writeBytes(f, ptr, size * nmemb);
  log_debug << "read " << size * nmemb << " bytes of data..." << std::endl;
  if (written < 0)
  {
    return 0;
  }
  else
  {
    return static_cast<size_t>(written);
  }
}

} // namespace

void
TransferStatus::abort()
{
  m_downloader.abort(id);
}

void
TransferStatus::update()
{
  m_downloader.update();
}

class Transfer final
{
private:
  Downloader& m_downloader;
  TransferId m_id;

  std::string m_url;
  CURL* m_handle;
  std::array<char, CURL_ERROR_SIZE> m_error_buffer;

  TransferStatusPtr m_status;
  std::unique_ptr<PHYSFS_file, int(*)(PHYSFS_File*)> m_fout;

public:
  Transfer(Downloader& downloader, TransferId id,
           const std::string& url,
           const std::string& outfile) :
    m_downloader(downloader),
    m_id(id),
    m_url(url),
    m_handle(),
    m_error_buffer({{'\0'}}),
    m_status(new TransferStatus(m_downloader, id)),
    m_fout(PHYSFS_openWrite(outfile.c_str()), PHYSFS_close)
  {
    if (!m_fout)
    {
      std::ostringstream out;
      out << "PHYSFS_openRead() failed: " << PHYSFS_getLastErrorCode();
      throw std::runtime_error(out.str());
    }

    m_handle = curl_easy_init();
    if (!m_handle)
    {
      throw std::runtime_error("curl_easy_init() failed");
    }
    else
    {
      curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str());
      curl_easy_setopt(m_handle, CURLOPT_USERAGENT, "SuperTux/" PACKAGE_VERSION " libcURL");

      curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, this);
      curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, &Transfer::on_data_wrap);

      curl_easy_setopt(m_handle, CURLOPT_ERRORBUFFER, m_error_buffer.data());
      curl_easy_setopt(m_handle, CURLOPT_NOSIGNAL, 1);
      curl_easy_setopt(m_handle, CURLOPT_FAILONERROR, 1);
      curl_easy_setopt(m_handle, CURLOPT_FOLLOWLOCATION, 1);

      curl_easy_setopt(m_handle, CURLOPT_NOPROGRESS, 0);
      curl_easy_setopt(m_handle, CURLOPT_PROGRESSDATA, this);
      curl_easy_setopt(m_handle, CURLOPT_PROGRESSFUNCTION, &Transfer::on_progress_wrap);
    }
  }

  ~Transfer()
  {
    curl_easy_cleanup(m_handle);
  }

  TransferStatusPtr get_status() const
  {
    return m_status;
  }

  const char* get_error_buffer() const
  {
    return m_error_buffer.data();
  }

  TransferId get_id() const
  {
    return m_id;
  }

  CURL* get_curl_handle() const
  {
    return m_handle;
  }

  std::string get_url() const
  {
    return m_url;
  }

  size_t on_data(void* ptr, size_t size, size_t nmemb)
  {
    PHYSFS_writeBytes(m_fout.get(), ptr, size * nmemb);
    return size * nmemb;
  }

  int on_progress(double dltotal, double dlnow,
                   double ultotal, double ulnow)
  {
    m_status->dltotal = static_cast<int>(dltotal);
    m_status->dlnow = static_cast<int>(dlnow);

    m_status->ultotal = static_cast<int>(ultotal);
    m_status->ulnow = static_cast<int>(ulnow);

    return 0;
  }

private:
  static size_t on_data_wrap(char* ptr, size_t size, size_t nmemb, void* userdata)
  {
    return static_cast<Transfer*>(userdata)->on_data(ptr, size, nmemb);
  }

  static int on_progress_wrap(void* userdata,
                              double dltotal, double dlnow,
                              double ultotal, double ulnow)
  {
    return static_cast<Transfer*>(userdata)->on_progress(dltotal, dlnow, ultotal, ulnow);
  }

private:
  Transfer(const Transfer&) = delete;
  Transfer& operator=(const Transfer&) = delete;
};

Downloader::Downloader() :
  m_multi_handle(),
  m_transfers(),
  m_next_transfer_id(1)
{
  curl_global_init(CURL_GLOBAL_ALL);
  m_multi_handle = curl_multi_init();
  if (!m_multi_handle)
  {
    throw std::runtime_error("curl_multi_init() failed");
  }
}

Downloader::~Downloader()
{
  for (auto& transfer : m_transfers)
  {
    curl_multi_remove_handle(m_multi_handle, transfer->get_curl_handle());
  }
  m_transfers.clear();

  curl_multi_cleanup(m_multi_handle);
  curl_global_cleanup();
}

void
Downloader::download(const std::string& url,
                     size_t (*write_func)(void* ptr, size_t size, size_t nmemb, void* userdata),
                     void* userdata)
{
  log_info << "Downloading " << url << std::endl;

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
  log_info << "download: " << url << " to " << filename << std::endl;
  std::unique_ptr<PHYSFS_file, int(*)(PHYSFS_File*)> fout(PHYSFS_openWrite(filename.c_str()),
                                                          PHYSFS_close);
  download(url, my_curl_physfs_write, fout.get());
}

void
Downloader::abort(TransferId id)
{
  auto it = std::find_if(m_transfers.begin(), m_transfers.end(),
                         [&id](const std::unique_ptr<Transfer>& rhs)
                         {
                           return id == rhs->get_id();
                         });
  if (it == m_transfers.end())
  {
    log_warning << "transfer not found: " << id << std::endl;
  }
  else
  {
    TransferStatusPtr status = (*it)->get_status();

    curl_multi_remove_handle(m_multi_handle, (*it)->get_curl_handle());
    m_transfers.erase(it);

    for (auto& callback : status->callbacks)
    {
      try
      {
        callback(false);
      }
      catch(const std::exception& err)
      {
        log_warning << "Illegal exception in Downloader: " << err.what() << std::endl;
      }
    }
  }
}

void
Downloader::update()
{
  // read data from the network
  CURLMcode ret;
  int running_handles;
  while ((ret = curl_multi_perform(m_multi_handle, &running_handles)) == CURLM_CALL_MULTI_PERFORM)
  {
    log_debug << "updating" << std::endl;
  }

  // check if any downloads got finished
  int msgs_in_queue;
  CURLMsg* msg;
  while ((msg = curl_multi_info_read(m_multi_handle, &msgs_in_queue)))
  {
    switch (msg->msg)
    {
      case CURLMSG_DONE:
        {
          CURLcode resultfromcurl = msg->data.result;
          log_info << "Download completed with " << resultfromcurl << std::endl;
          curl_multi_remove_handle(m_multi_handle, msg->easy_handle);

          auto it = std::find_if(m_transfers.begin(), m_transfers.end(),
                                 [&msg](const std::unique_ptr<Transfer>& rhs) {
                                   return rhs->get_curl_handle() == msg->easy_handle;
                                 });
          assert(it != m_transfers.end());
          TransferStatusPtr status = (*it)->get_status();
          status->error_msg = (*it)->get_error_buffer();
          m_transfers.erase(it);

          if (resultfromcurl == CURLE_OK)
          {
            bool success = true;
            for (auto& callback : status->callbacks)
            {
              try
              {
                callback(success);
              }
              catch(const std::exception& err)
              {
                success = false;
                log_warning << "Exception in Downloader: " << err.what() << std::endl;
                status->error_msg = err.what();
              }
            }
          }
          else
          {
            log_warning << "Error: " << curl_easy_strerror(resultfromcurl) << std::endl;
            for (auto& callback : status->callbacks)
            {
              try
              {
                callback(false);
              }
              catch(const std::exception& err)
              {
                log_warning << "Illegal exception in Downloader: " << err.what() << std::endl;
              }
            }
          }
        }
        break;

      default:
        log_warning << "unhandled cURL message: " << msg->msg << std::endl;
        break;
    }
  }
}

TransferStatusPtr
Downloader::request_download(const std::string& url, const std::string& outfile)
{
  log_info << "request_download: " << url << std::endl;
  auto transfer = std::make_unique<Transfer>(*this, m_next_transfer_id++, url, outfile);
  curl_multi_add_handle(m_multi_handle, transfer->get_curl_handle());
  m_transfers.push_back(std::move(transfer));
  return m_transfers.back()->get_status();
}

#endif

/* EOF */
