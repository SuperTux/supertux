//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "util/file_system.hpp"

#include <boost/filesystem.hpp>
#include <boost/version.hpp>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#if defined(_WIN32)
  #include <windows.h>
  #include <shellapi.h>
#else
  #include <cstdlib>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <SDL.h>

#include "gui/dialog.hpp"
#include "util/log.hpp"
#include "util/string_util.hpp"

namespace fs = boost::filesystem;

namespace FileSystem {

bool exists(const std::string& path)
{
  fs::path location(path);
  boost::system::error_code ec;

  // If we get an error (such as "Permission denied"), then ignore it
  // and pretend that the path doesn't exist.
  return fs::exists(location, ec);
}

bool is_directory(const std::string& path)
{
  fs::path location(path);
  return fs::is_directory(location);
}

void mkdir(const std::string& directory)
{
  fs::path location(directory);
  if (!fs::create_directory(location))
  {
    throw std::runtime_error("failed to create directory: "  + directory);
  }
}

std::string dirname(const std::string& filename)
{
  std::string::size_type p = filename.find_last_of('/');
  if (p == std::string::npos)
    p = filename.find_last_of('\\');
  if (p == std::string::npos)
    return "./";

  return filename.substr(0, p+1);
}

std::string basename(const std::string& filename)
{
  std::string::size_type p = filename.find_last_of('/');
  if (p == std::string::npos)
    p = filename.find_last_of('\\');
  if (p == std::string::npos)
    return filename;

  return filename.substr(p+1, filename.size()-p-1);
}

std::string relpath(const std::string& filename, const std::string& basedir)
{
#if BOOST_VERSION >= 106000
  return fs::relative(filename, basedir).string();
#else
  fs::path from = basedir;
  fs::path to = filename;

  // Taken from https://stackoverflow.com/a/29221546

  // Start at the root path and while they are the same then do nothing then when they first
  // diverge take the entire from path, swap it with '..' segments, and then append the remainder of the to path.
  fs::path::const_iterator fromIter = from.begin();
  fs::path::const_iterator toIter = to.begin();

  // Loop through both while they are the same to find nearest common directory
  while (fromIter != from.end() && toIter != to.end() && (*toIter) == (*fromIter))
  {
    ++toIter;
    ++fromIter;
  }

  // Replace from path segments with '..' (from => nearest common directory)
  fs::path finalPath;
  while (fromIter != from.end())
  {
    finalPath /= "..";
    ++fromIter;
  }

  // Append the remainder of the to path (nearest common directory => to)
  while (toIter != to.end())
  {
    finalPath /= *toIter;
    ++toIter;
  }

  return finalPath.string();
#endif
}

std::string extension(const std::string& filename)
{
  std::string::size_type p = filename.find_last_of('.');
  if (p == std::string::npos)
    return "";

  return filename.substr(p);
}

std::string strip_extension(const std::string& filename)
{
  std::string::size_type p = filename.find_last_of('.');
  if (p == std::string::npos)
    return filename;

  return filename.substr(0, p);
}

std::string normalize(const std::string& filename)
{
  std::vector<std::string> path_stack;

  const char* p = filename.c_str();

  while (true) {
    while (*p == '/' || *p == '\\') {
      p++;
      continue;
    }

    const char* pstart = p;
    while (*p != '/' && *p != '\\' && *p != 0) {
      ++p;
    }

    size_t len = p - pstart;
    if (len == 0)
      break;

    std::string pathelem(pstart, p-pstart);
    if (pathelem == ".")
      continue;

    if (pathelem == "..") {
      if (path_stack.empty()) {

        log_warning << "Invalid '..' in path '" << filename << "'" << std::endl;
        // push it into the result path so that the user sees his error...
        path_stack.push_back(pathelem);
      } else {
        path_stack.pop_back();
      }
    } else {
      path_stack.push_back(pathelem);
    }
  }

  // construct path
  std::ostringstream result;
  for (std::vector<std::string>::iterator i = path_stack.begin();
       i != path_stack.end(); ++i) {
    result << '/' << *i;
  }
  if (path_stack.empty())
    result << '/';

  return result.str();
}

std::string join(const std::string& lhs, const std::string& rhs)
{
  if (lhs.empty())
  {
    return rhs;
  }
  else if (rhs.empty())
  {
    return lhs + "/";
  }
  else if (lhs.back() == '/' && rhs.front() != '/')
  {
    return lhs + rhs;
  }
  else if (lhs.back() != '/' && rhs.front() == '/')
  {
    return lhs + rhs;
  }
  else if (lhs.back() == '/' && rhs.front() == '/')
  {
    return lhs + rhs.substr(1);
  }
  else
  {
    return lhs + "/" + rhs;
  }
}

bool remove(const std::string& path)
{
  fs::path location(path);
  return fs::remove(location);
}

void open_path(const std::string& path)
{
#ifdef __ANDROID__
  // SDL >= 2.0.14 is strictly required for Android
  SDL_OpenURL(("file://" + path).c_str());
#elif defined(_WIN32) || defined (_WIN64)
  ShellExecute(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif defined(__EMSCRIPTEN__)
  emscripten_run_script(("window.supertux_download('" + path + "');").c_str());
#else
  #if defined(__APPLE__)
  std::string cmd = "open \"" + path + "\"";
  #else
  std::string cmd = "xdg-open \"" + path + "\"";
  #endif

  int ret = system(cmd.c_str());
  if (ret < 0)
  {
    log_fatal << "failed to spawn: " << cmd << std::endl;
  }
  else if (ret > 0)
  {
    log_fatal << "error " << ret << " while executing: " << cmd << std::endl;
  }
#endif
}

void open_url(const std::string& url)
{
#if SDL_VERSION_ATLEAST(2,0,14)
  SDL_OpenURL(url.c_str());
#elif defined(__EMSCRIPTEN__)
  emscripten_run_script(("window.open('" + url + "');").c_str());
#else
  open_path(url);
#endif
}

} // namespace FileSystem

/* EOF */
