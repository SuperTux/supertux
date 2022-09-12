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

#include "addon/downloader.hpp"

#include <algorithm>
#include <array>
#include <assert.h>
#include <memory>
#include <physfs.h>
#include <sstream>
#include <stdexcept>
#include <version.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/string_util.hpp"

namespace {

// This one is necessary for a download function
size_t my_curl_string_append(void* ptr, size_t size, size_t nmemb, void* userdata)
{
  std::string& s = *static_cast<std::string*>(userdata);
  std::string buf(static_cast<char*>(ptr), size * nmemb);
  s += buf;
  log_debug << "read " << size * nmemb << " bytes of data..." << std::endl;
  return size * nmemb;
}

#ifndef EMSCRIPTEN
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
#endif

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
#ifndef EMSCRIPTEN
  CURL* m_handle;
  std::array<char, CURL_ERROR_SIZE> m_error_buffer;
#endif

  TransferStatusPtr m_status;
#ifndef EMSCRIPTEN
  std::unique_ptr<PHYSFS_file, int(*)(PHYSFS_File*)> m_fout;
#endif

public:
  Transfer(Downloader& downloader, TransferId id,
           const std::string& url,
           const std::string& outfile) :
    m_downloader(downloader),
    m_id(id),
    m_url(url),
#ifndef EMSCRIPTEN
    m_handle(),
    m_error_buffer({{'\0'}}),
#endif
    m_status(new TransferStatus(m_downloader, id))
#ifndef EMSCRIPTEN
    ,
    m_fout(PHYSFS_openWrite(outfile.c_str()), PHYSFS_close)
#endif
  {
#ifndef EMSCRIPTEN
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
#else
    // Avoid code injection from funny callers
    auto url_clean = StringUtil::replace_all(StringUtil::replace_all(url, "\\", "\\\\"), "'", "\\'");
    auto path_clean = StringUtil::replace_all(StringUtil::replace_all(FileSystem::join(std::string(PHYSFS_getWriteDir()), outfile), "\\", "\\\\"), "'", "\\'");
    emscripten_run_script(("supertux_xhr_download(" + std::to_string(m_id) + ", '" + url_clean + "', '" + path_clean + "');").c_str());
#endif
  }

  ~Transfer()
  {
#ifndef EMSCRIPTEN
    curl_easy_cleanup(m_handle);
#endif
  }

  TransferStatusPtr get_status() const
  {
    return m_status;
  }

#ifndef EMSCRIPTEN
  const char* get_error_buffer() const
  {
    return m_error_buffer.data();
  }
#endif

  TransferId get_id() const
  {
    return m_id;
  }

#ifndef EMSCRIPTEN
  CURL* get_curl_handle() const
  {
    return m_handle;
  }
#endif

  std::string get_url() const
  {
    return m_url;
  }

#ifndef EMSCRIPTEN
  size_t on_data(void* ptr, size_t size, size_t nmemb)
  {
    PHYSFS_writeBytes(m_fout.get(), ptr, size * nmemb);
    return size * nmemb;
  }
#endif

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
#ifndef EMSCRIPTEN
  static size_t on_data_wrap(char* ptr, size_t size, size_t nmemb, void* userdata)
  {
    return static_cast<Transfer*>(userdata)->on_data(ptr, size, nmemb);
  }
#endif

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
#ifndef EMSCRIPTEN
  m_multi_handle(),
#endif
  m_transfers(),
  m_next_transfer_id(1)
{
#ifndef EMSCRIPTEN
  curl_global_init(CURL_GLOBAL_ALL);
  m_multi_handle = curl_multi_init();
  if (!m_multi_handle)
  {
    throw std::runtime_error("curl_multi_init() failed");
  }
#endif
}

Downloader::~Downloader()
{
#ifndef EMSCRIPTEN
  for (auto& transfer : m_transfers)
  {
    curl_multi_remove_handle(m_multi_handle, transfer->get_curl_handle());
  }
#endif
  m_transfers.clear();

#ifndef EMSCRIPTEN
  curl_multi_cleanup(m_multi_handle);
  curl_global_cleanup();
#endif
}

void
Downloader::download(const std::string& url,
                     size_t (*write_func)(void* ptr, size_t size, size_t nmemb, void* userdata),
                     void* userdata)
{
  log_info << "Downloading " << url << std::endl;

#ifndef EMSCRIPTEN
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
#else
  log_warning << "Direct download not yet implemented for Emscripten" << std::endl;
  // FUTURE MAINTAINERS: If this needs to be implemented, take a look at
  // emscripten_wget(), emscripten_async_wget(), emscripten_wget_data() and
  // emscripten_async_wget_data():
  // https://emscripten.org/docs/api_reference/emscripten.h.html#c.emscripten_wget
#endif
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
#ifndef EMSCRIPTEN
  log_info << "download: " << url << " to " << filename << std::endl;
  std::unique_ptr<PHYSFS_file, int(*)(PHYSFS_File*)> fout(PHYSFS_openWrite(filename.c_str()),
                                                          PHYSFS_close);
  download(url, my_curl_physfs_write, fout.get());
#else
  log_warning << "Direct download not yet implemented for Emscripten" << std::endl;
  // FUTURE MAINTAINERS: If this needs to be implemented, take a look at
  // emscripten_wget(), emscripten_async_wget(), emscripten_wget_data() and
  // emscripten_async_wget_data():
  // https://emscripten.org/docs/api_reference/emscripten.h.html#c.emscripten_wget
#endif
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

#ifndef EMSCRIPTEN
    curl_multi_remove_handle(m_multi_handle, (*it)->get_curl_handle());
#endif
    m_transfers.erase(it);

    for (const auto& callback : status->callbacks)
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
#ifndef EMSCRIPTEN
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
            for (const auto& callback : status->callbacks)
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
            for (const auto& callback : status->callbacks)
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
#endif
}

TransferStatusPtr
Downloader::request_download(const std::string& url, const std::string& outfile)
{
  log_info << "request_download: " << url << std::endl;
  auto transfer = std::make_unique<Transfer>(*this, m_next_transfer_id++, url, outfile);
#ifndef EMSCRIPTEN
  curl_multi_add_handle(m_multi_handle, transfer->get_curl_handle());
#endif
  m_transfers.push_back(std::move(transfer));
  return m_transfers.back()->get_status();
}

#ifdef EMSCRIPTEN
void
Downloader::onDownloadProgress(int id, int loaded, int total)
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
    (*it)->on_progress(static_cast<double>(loaded), static_cast<double>(total), 0.0, 0.0);
  }
}

void
Downloader::onDownloadFinished(int id)
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
    for (const auto& callback : (*it)->get_status()->callbacks)
    {
      try
      {
        callback(true);
      }
      catch(const std::exception& err)
      {
        log_warning << "Exception in Downloader: " << err.what() << std::endl;
      }
    }
  }
}

void
Downloader::onDownloadError(int id)
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
    for (const auto& callback : (*it)->get_status()->callbacks)
    {
      try
      {
        callback(false);
      }
      catch(const std::exception& err)
      {
        log_warning << "Exception in Downloader: " << err.what() << std::endl;
      }
    }
  }
}

void
Downloader::onDownloadAborted(int id)
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
    for (const auto& callback : (*it)->get_status()->callbacks)
    {
      try
      {
        callback(false);
      }
      catch(const std::exception& err)
      {
        log_warning << "Exception in Downloader: " << err.what() << std::endl;
      }
    }
  }
}
#endif

/* EOF */
