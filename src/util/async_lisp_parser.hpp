//  SuperTux
//  Copyright (C) 2016 Tobias Markus <tobbi.bugs@googlemail.com>
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

#ifndef HEADER_SUPERTUX_UTIL_ASYNC_LISP_PARSE_HPP
#define HEADER_SUPERTUX_UTIL_ASYNC_LISP_PARSE_HPP

#include <functional>
#include <future>
#include <vector>

#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

class AsyncWorkload;
class LispParserWorkload;
class ListFilesWorkload;

namespace {
  /**
   * Type definitions
   */
  // A list containing different workloads
  typedef std::vector<AsyncWorkload*> Workloads;
}

class AsyncWorkload
{
public:
  // Enumeration describing the kind of work load
  typedef enum { PARSE_LISP, LIST_FILES } WorkloadType;

private:
  WorkloadType m_type;
  std::function<void ()> on_completed;

public:
  AsyncWorkload(const WorkloadType& type) :
    m_type(type),
    on_completed()
  {
  }

  virtual ~AsyncWorkload()
  {
  }

  const WorkloadType& get_type() const {
    return m_type;
  }

  void set_on_completed(std::function<void()> handler)
  {
    on_completed = handler;
  }

  void completed()
  {
    on_completed();
  }
};

class LispParserWorkload : public AsyncWorkload
{
  typedef std::function<void(const ReaderDocument& doc)> LispParserCallback;

  const std::string& m_path;
  const LispParserCallback& m_callback;

public:
  LispParserWorkload(const std::string& path,
                     const LispParserCallback& callback) :
    AsyncWorkload(PARSE_LISP),
    m_path(path),
    m_callback(callback)
  {
  }

  const std::string& get_path() const {
    return m_path;
  }

  const LispParserCallback& get_callback() const {
    return m_callback;
  }

};

class ListFilesWorkload : public AsyncWorkload
{
  typedef std::function<void(char** files)> ListFilesCallback;

  const std::string& m_path;
  const ListFilesCallback& m_callback;

public:
  ListFilesWorkload(const std::string& path,
                    const ListFilesCallback& callback) :
    AsyncWorkload(LIST_FILES),
    m_path(path),
    m_callback(callback)
  {
  }

const std::string& get_path() const {
  return m_path;
}
const ListFilesCallback& get_callback() const {
  return m_callback;
}
};

class Workers
{
public:
  static void parser(LispParserWorkload* workload)
  {
    auto path = workload->get_path();
    auto callback = workload->get_callback();
    if(path.empty())
    {
      return;
    }
    auto document = ReaderDocument::parse(path);
    callback(document);
    workload->completed();
  }

  static void enumerator(ListFilesWorkload* workload)
  {
    auto path = workload->get_path();
    auto callback = workload->get_callback();
    std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
      rc(PHYSFS_enumerateFiles(path.c_str()), PHYSFS_freeList);
    callback(rc.get());
    workload->completed();
  }
};

class AsyncWorker
{
private:
  Workloads* m_requests;
  size_t m_number_done;

private:
  void report_progress() {
    m_number_done++;
  }

public:
  AsyncWorker(Workloads* requests):
    m_requests(requests),
    m_number_done()
  {
  }

  void start_working() {
    for(auto& request: *m_requests)
    {
      // Add an onCompleted event handler:
      request->set_on_completed([this] () {
        report_progress();
      });

      switch(request->get_type())
      {
        case AsyncWorkload::PARSE_LISP:
          std::async(Workers::parser,
                     static_cast<LispParserWorkload*>(request));
          break;
        case AsyncWorkload::LIST_FILES:
          std::async(Workers::enumerator,
                     static_cast<ListFilesWorkload*>(request));
          break;
      }
    }
  }

  size_t get_num_done() const {
    return m_number_done;
  }

  bool was_completed() const {
    return get_num_done() == m_requests->size();
  }
};
#endif
