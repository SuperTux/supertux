//  SuperTux
//  Copyright (C) 2026 MatusGuy
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

#pragma once

#include <fstream>

#include <physfs.h>

#include "cheevos/cheevos.hpp"
#include "supertux/profile.hpp"
#include "addon/addon.hpp"
#include "physfs/util.hpp"
#include "physfs/ifile_stream.hpp"
#include "physfs/ofile_stream.hpp"

class CheevoManager
{
public:
  CheevoManager();

  void init();
  void deinit();
  void unlock(CheevoId cheevo, const Profile& profile, const Addon* addon = nullptr);
  const std::vector<bool>& get_unlocked(const Profile& profile, const Addon* addon = nullptr);

  // Local cheevo provider
  void init_local();
  void deinit_local();
  void unlock_local(CheevoId cheevo, const Profile& profile, const Addon* addon = nullptr);
  const std::vector<bool>& get_unlocked_local(const Profile& profile, const Addon* addon = nullptr);

private:
  struct ProfileCheevoData {
    // Local cheevo provider members
    std::unique_ptr<OFileStream> file;
    std::vector<bool> unlocked_local;
  };

  std::unordered_map<int, ProfileCheevoData> m_profiledata;
};

extern CheevoManager g_cheevos;
