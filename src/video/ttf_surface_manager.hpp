//  SuperTux
//  Copyright (C) 2016 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_VIDEO_TTF_SURFACE_MANAGER_HPP
#define HEADER_SUPERTUX_VIDEO_TTF_SURFACE_MANAGER_HPP

#include <tuple>
#include <map>
#include <string>

#include "util/currenton.hpp"
#include "video/color.hpp"
#include "video/surface_ptr.hpp"

class TTFFont;

class TTFSurfaceManager : public Currenton<TTFSurfaceManager>
{
public:
  TTFSurfaceManager();

  SurfacePtr create_surface(const TTFFont& font, const std::string& text);

private:
  void cache_cleanup_step();

private:
  struct CacheEntry
  {
    CacheEntry() : surface(), last_access() {}
    CacheEntry(const SurfacePtr& s);

    SurfacePtr surface;
    float last_access;
  };

private:
  using Key = std::tuple<void*, std::string>;
  std::map<Key, CacheEntry> m_cache;

  std::map<Key, CacheEntry>::iterator m_cache_iter;

private:
  TTFSurfaceManager(const TTFSurfaceManager&) = delete;
  TTFSurfaceManager& operator=(const TTFSurfaceManager&) = delete;
};

#endif

/* EOF */
