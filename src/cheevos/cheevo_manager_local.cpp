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
#include <cstring>

#include "util/log.hpp"

static void
read_unlocked_cheevos(std::istream& stream, std::vector<bool>& out)
{
  std::size_t /*size, startpos,*/ cheevobytes_size = 0;
  std::uint16_t cheevocount = 0;
  char* cheevobytes = nullptr;

  if (stream.eof())
    return;

  stream.read(reinterpret_cast<char*>(&cheevocount), sizeof(cheevocount));
  cheevobytes_size = std::ceil(cheevocount / 8.f);

  cheevobytes = new char[cheevobytes_size];
  std::memset(cheevobytes, 0, cheevobytes_size);
  // stream.seekg(startpos);
  stream.read(cheevobytes, cheevobytes_size);

  out.reserve(cheevocount * 8);
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
  // TODO: update with profile changes
  // TODO: add header to file format

  for (auto& [id, data] : m_profiledata)
  {
    data.filename = Profile::get_basedir(id) + "/cheevos";

    try
    {
      IFileStream istream(data.filename);
      read_unlocked_cheevos(istream, data.unlocked_local);
    }
    catch (std::runtime_error&)
    {
      // File probably does not exist. Create it now.
      // This is here in order to prevent lag spikes.
      PHYSFS_File* file = PHYSFS_openWrite(data.filename.c_str());
      if (file == nullptr)
      {
        std::stringstream msg;
        msg << "Couldn't open file '" << data.filename << "': "
            << physfsutil::get_last_error();
        throw std::runtime_error(msg.str());
      }

      PHYSFS_close(file);
    }
  }
}

void
CheevoManager::deinit_local()
{
}

void
CheevoManager::unlock_local(CheevoId cheevo, const Profile& profile, const Addon* addon)
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

  PHYSFS_File* file = PHYSFS_openWrite(it->second.filename.c_str());
  if (file == nullptr)
  {
    std::stringstream msg;
    msg << "Couldn't open file '" << it->second.filename << "': "
        << physfsutil::get_last_error();
    throw std::runtime_error(msg.str());
  }

  PHYSFS_seek(file, 0);

  const std::uint16_t cheevocount = std::min(unlocked.size(), static_cast<std::size_t>(UINT16_MAX));
  PHYSFS_writeBytes(file, &cheevocount, sizeof(cheevocount));

  char cheevobyte = 0, bit = 0;
  for (int i = 0; i < cheevocount; ++i)
  {
    bit = i % 8;
    if (unlocked[i])
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
  PHYSFS_close(file);
}

std::vector<bool> const&
CheevoManager::get_unlocked_local(const Profile& profile, const Addon* addon)
{
  auto it = m_profiledata.find(profile.get_id());
  if (it == m_profiledata.end())
  {
    log_warning << "Unable to unlock cheevo " " in profile " << profile.get_id() << ": Local cheevo store unavailable.";
    return {};
  }

  return it->second.unlocked_local;
}

void
CheevoManager::reset_all_local(const Profile& profile, const Addon* addon)
{
  auto it = m_profiledata.find(profile.get_id());
  if (it == m_profiledata.end())
  {
    log_warning << "Unable to unlock cheevo " " in profile " << profile.get_id() << ": Local cheevo store unavailable.";
    return;
  }

  CheevoProfileData& profiledata = it->second;
  profiledata.unlocked_local.erase(profiledata.unlocked_local.begin(),
                                   profiledata.unlocked_local.end());

  PHYSFS_File* file = PHYSFS_openWrite(profiledata.filename.c_str());
  if (file == nullptr)
  {
    std::stringstream msg;
    msg << "Couldn't open file '" << profiledata.filename << "': "
        << physfsutil::get_last_error();
    throw std::runtime_error(msg.str());
  }

  // Write nothing

  PHYSFS_flush(file);
  PHYSFS_close(file);
}
