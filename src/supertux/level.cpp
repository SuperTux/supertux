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

#include "supertux/level.hpp"

#include "badguy/goldbomb.hpp"
#include "object/bonus_block.hpp"
#include "object/coin.hpp"
#include "physfs/util.hpp"
#include "supertux/sector.hpp"
#include "trigger/secretarea_trigger.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/writer.hpp"

#include <physfs.h>
#include <numeric>

#include <boost/algorithm/string/predicate.hpp>

Level* Level::s_current = nullptr;

Level::Level(bool worldmap) :
  m_is_worldmap(worldmap),
  m_name("noname"),
  m_author("SuperTux Player"),
  m_contact(),
  m_license(),
  m_filename(),
  m_sectors(),
  m_stats(),
  m_note(),
  m_target_time(),
  m_tileset("images/tiles.strf"),
  m_suppress_pause_menu(),
  m_is_in_cutscene(false),
  m_skip_cutscene(false)
{
  s_current = this;
}

Level::~Level()
{
  m_sectors.clear();
}

void
Level::save(std::ostream& stream)
{
  Writer writer(stream);
  save(writer);
}

void
Level::save(const std::string& filepath, bool retry)
{
  //FIXME: It tests for directory in supertux/data, but saves into .supertux2.
  try {
    { // make sure the level directory exists
      std::string dirname = FileSystem::dirname(filepath);
      if (!PHYSFS_exists(dirname.c_str()))
      {
        if (!PHYSFS_mkdir(dirname.c_str()))
        {
          std::ostringstream msg;
          msg << "Couldn't create directory for level '"
              << dirname << "': " <<PHYSFS_getLastErrorCode();
          throw std::runtime_error(msg.str());
        }
      }

      if (!physfsutil::is_directory(dirname))
      {
        std::ostringstream msg;
        msg << "Level path '" << dirname << "' is not a directory";
        throw std::runtime_error(msg.str());
      }
    }

    Writer writer(filepath);
    save(writer);
    log_warning << "Level saved as " << filepath << "." 
                << (boost::algorithm::ends_with(filepath, "~") ? " [Autosave]" : "")
                << std::endl;
  } catch(std::exception& e) {
    if (retry) {
      std::stringstream msg;
      msg << "Problem when saving level '" << filepath << "': " << e.what();
      throw std::runtime_error(msg.str());
    } else {
      log_warning << "Failed to save the level, retrying..." << std::endl;
      { // create the level directory again
        std::string dirname = FileSystem::dirname(filepath);
        if (!PHYSFS_mkdir(dirname.c_str()))
        {
          std::ostringstream msg;
          msg << "Couldn't create directory for level '"
              << dirname << "': " <<PHYSFS_getLastErrorCode();
          throw std::runtime_error(msg.str());
        }
      }
      save(filepath, true);
    }
  }
}

void
Level::save(Writer& writer)
{
  writer.start_list("supertux-level");
  // Starts writing to supertux level file. Keep this at the very beginning.

  writer.write("version", 3);
  writer.write("name", m_name, true);
  writer.write("author", m_author, false);
  if(!m_note.empty()){
    writer.write("note",m_note,false);
  }
  if (!m_contact.empty()) {
    writer.write("contact", m_contact, false);
  }
  if (!m_license.empty()) {
    writer.write("license", m_license, false);
  }
  if (m_target_time != 0.0f){
    writer.write("target-time", m_target_time);
  }
  if(m_suppress_pause_menu) {
    writer.write("suppress-pause-menu", m_suppress_pause_menu);
  }

  for (auto& sector : m_sectors) {
    sector->save(writer);
  }

  if (m_tileset != "images/tiles.strf")
    writer.write("tileset", m_tileset, false);

  // Ends writing to supertux level file. Keep this at the very end.
  writer.end_list("supertux-level");
}

void
Level::add_sector(std::unique_ptr<Sector> sector)
{
  Sector* test = get_sector(sector->get_name());
  if (test != nullptr) {
    throw std::runtime_error("Trying to add 2 sectors with same name");
  } else {
    m_sectors.push_back(std::move(sector));
  }
}

Sector*
Level::get_sector(const std::string& name_) const
{
  auto _sector = std::find_if(m_sectors.begin(), m_sectors.end(), [name_] (const std::unique_ptr<Sector>& sector) {
    return sector->get_name() == name_;
  });
  if(_sector == m_sectors.end())
    return nullptr;
  return _sector->get();
}

size_t
Level::get_sector_count() const
{
  return m_sectors.size();
}

Sector*
Level::get_sector(size_t num) const
{
  return m_sectors.at(num).get();
}

int
Level::get_total_coins() const
{
  int total_coins = 0;
  for (auto const& sector : m_sectors) {
    for (const auto& o: sector->get_objects()) {
      auto coin = dynamic_cast<Coin*>(o.get());
      if (coin)
      {
        total_coins++;
        continue;
      }
      auto block = dynamic_cast<BonusBlock*>(o.get());
      if (block)
      {
        if (block->get_contents() == BonusBlock::Content::COIN)
        {
          total_coins += block->get_hit_counter();
          continue;
        } else if (block->get_contents() == BonusBlock::Content::RAIN ||
                   block->get_contents() == BonusBlock::Content::EXPLODE)
        {
          total_coins += 10;
          continue;
        }
      }
      auto goldbomb = dynamic_cast<GoldBomb*>(o.get());
      if (goldbomb)
        total_coins += 10;
    }
  }
  return total_coins;
}

int
Level::get_total_badguys() const
{
  int total_badguys = 0;
  for (auto const& sector : m_sectors) {
    total_badguys += sector->get_object_count<BadGuy>([] (const BadGuy& badguy) {
      return badguy.m_countMe;
    });
  }
  return total_badguys;
}

int
Level::get_total_secrets() const
{
  auto get_secret_count = [](int accumulator, const std::unique_ptr<Sector>& sector) {
    return accumulator + sector->get_object_count<SecretAreaTrigger>();
  };
  return std::accumulate(m_sectors.begin(), m_sectors.end(), 0, get_secret_count);
}

void
Level::reactivate()
{
  s_current = this;
}

/* EOF */
