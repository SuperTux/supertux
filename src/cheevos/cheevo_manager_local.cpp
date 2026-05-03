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

#include "cheevo_manager.hpp"

#include <cmath>
#include <cinttypes>
#include <sstream>

#include "util/log.hpp"

static void
read_unlocked_cheevos(std::istream& stream, std::vector<bool>& out)
{
  std::size_t size, startpos;
  char* cheevobytes = nullptr;

  startpos = stream.tellg();
  stream.seekg(std::ios::end);
  size = stream.gcount();

  cheevobytes = new char[size];
  stream.seekg(startpos);
  stream.read(cheevobytes, size);

  out.reserve(size * 8);
  for (int i = 0; i < out.capacity(); ++i)
  {
    out.push_back((cheevobytes[i / 8] & 1 << (i % 8)) != 0);
  }

  delete[] cheevobytes;
}

void
CheevoManager::init_local()
{
  // TODO: init addon files
  // TODO: add header to file format

  for (auto& [id, data] : m_profiledata)
  {
    std::string path = Profile::get_basedir(id) + "/cheevos";

    try
    {
      IFileStream istream(path);
      read_unlocked_cheevos(istream, data.unlocked_local);
    }
    catch (std::runtime_error&)
    {
      // File does not exist.
    }

    data.file = PHYSFS_openWrite(path.c_str());
    if (data.file == nullptr)
    {
      std::stringstream msg;
      msg << "Couldn't open file '" << path << "': "
          << physfsutil::get_last_error();
      throw std::runtime_error(msg.str());
    }
  }
}

void
CheevoManager::deinit_local()
{
  for (auto& [id, data] : m_profiledata) {
    if (data.file)
      PHYSFS_close(data.file);
  }
}

void CheevoManager::unlock_local(CheevoId cheevo, const Profile& profile, const Addon* addon)
{
  auto it = m_profiledata.find(profile.get_id());
  if (it == m_profiledata.end())
  {
    log_warning << "Unable to unlock cheevo " " in profile " << profile.get_id() << ": Local cheevo store unavailable.";
    return;
  }

  std::vector<bool>& unlocked = it->second.unlocked_local;
  if (cheevo >= unlocked.size())
    unlocked.resize(cheevo + 1);
  unlocked[cheevo] = true;

  PHYSFS_File* file = it->second.file;

  PHYSFS_seek(file, 0);

  char cheevobyte = 0, bit = 0;
  for (int i = 0; i < unlocked.size(); ++i)
  {
    bit = i % 8;
    cheevobyte |= 1 << bit;

    if (bit == 7)
    {
      // Last bit
      PHYSFS_writeBytes(file, &cheevobyte, 1);
      cheevobyte = 0;
    }
  }

  // Write leftover bits
  if (bit != 7)
  {
    PHYSFS_writeBytes(file, &cheevobyte, 1);
  }

  PHYSFS_flush(file);
}

const std::vector<bool>& CheevoManager::get_unlocked_local(const Profile& profile, const Addon* addon)
{
  auto it = m_profiledata.find(profile.get_id());
  if (it == m_profiledata.end())
  {
    log_warning << "Unable to unlock cheevo " " in profile " << profile.get_id() << ": Local cheevo store unavailable.";
    return {};
  }

  return it->second.unlocked_local;
}
