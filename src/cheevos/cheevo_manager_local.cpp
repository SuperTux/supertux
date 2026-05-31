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
#include <cstring>
#include <optional>

#include "util/reader_mapping.hpp"
#include "util/reader_document.hpp"
#include "util/writer.hpp"
#include "util/log.hpp"

static void
read_unlocked_cheevos(ReaderDocument& doc, CheevosUnlocked& out)
{
  ReaderObject root = doc.get_root();
  if (root.get_name() != "supertux-cheevos")
  {
    throw std::runtime_error("Cheevos file is not of type supertux-cheevos.");
  }

  ReaderMapping mapping = root.get_mapping();
  std::optional<ReaderMapping> cheevos;
  if (!mapping.get("cheevos", cheevos))
  {
    throw std::runtime_error("Cheevos file does not contain cheevos list.");
  }

  auto iter = cheevos->get_iter();
  while (iter.next()) {
    CheevoUnlocked unlocked;

    std::string timestring;
    cheevos->get(iter.get_key().c_str(), timestring);

    if constexpr (sizeof(std::time_t) == 8)
      unlocked.time = std::stoll(timestring);
    else
      unlocked.time = std::stol(timestring);

    out.emplace(iter.get_key(), unlocked);
  }
}

static void
write_unlocked_cheevos(Writer& writer, CheevosUnlocked const& in) {
  writer.start_list("supertux-cheevos");
  writer.start_list("cheevos");

  for (auto& [id, cheevo] : in) {
    if (!cheevo.unlocked())
      continue;

    writer.write(id, std::to_string(cheevo.time));
  }

  writer.end_list("cheevos");
  writer.end_list("supertux-cheevos");
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
      auto doc = ReaderDocument::from_file(data.filename);
      read_unlocked_cheevos(doc, data.unlocked_local);
    }
    catch (std::runtime_error&)
    {
      // File probably does not exist. Create it now in order to prevent future lag spikes.

      Writer writer(data.filename);
      write_unlocked_cheevos(writer, data.unlocked_local);
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
    log_warning << "Unable to unlock cheevo " << cheevo << " in profile " << profile.get_id() << ": Local cheevo store unavailable.";
    return;
  }

  CheevosUnlocked& unlocked = it->second.unlocked_local;
  CheevoUnlocked& data = unlocked.at(cheevo);
  data.time = std::time(nullptr);

  try
  {
    Writer writer(it->second.filename);
    write_unlocked_cheevos(writer, unlocked);
  }
  catch (std::runtime_error& e)
  {
    log_warning << "Unable to unlock cheevo " << cheevo << " in profile " << profile.get_id() << ": " << e.what();
    return;
  }
}

CheevosUnlocked const&
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

  try
  {
    Writer writer(it->second.filename);
    write_unlocked_cheevos(writer, profiledata.unlocked_local);
  }
  catch (std::runtime_error& e)
  {
    log_warning << "Unable to reset cheevos in profile " << profile.get_id() << ": " << e.what();
    return;
  }
}
