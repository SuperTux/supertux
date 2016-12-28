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

#include <functional>
#include <future>

#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

namespace {
  static void parser_thread(const std::string& path,
                            const std::function<void(const ReaderDocument& doc)>& callback)
  {
    if(path.empty())
    {
      return;
    }
    auto document = ReaderDocument::parse(path);
    callback(document);
  }

  static void enumerate_thread(const std::string& path,
                               const std::function<void(char** files)>& callback)
  {
    std::unique_ptr<char*, decltype(&PHYSFS_freeList)>
      rc(PHYSFS_enumerateFiles(path.c_str()), PHYSFS_freeList);
    callback(rc.get());
  }
}

class AsyncParser
{
public:
  static void parse(const std::string& path,
                    const std::function<void (const ReaderDocument& doc)>& callback)
  {
    std::async(parser_thread, path, callback);
  }

  static void enumerate_files(const std::string& path,
                              const std::function<void (char** files)> callback)
  {
    std::async(enumerate_thread, path, callback);
  }
};

class AsyncParserWorkload
{

};
