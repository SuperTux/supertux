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
#include "physfs/ifile_streambuf.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/tile_set.hpp"
#include "trigger/secretarea_trigger.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/writer.hpp"

#include <sstream>
#include <stdexcept>

using namespace std;

Level* Level::_current = 0;

Level::Level() :
  name("noname"),
  author("Mr. X"),
  contact(),
  license(),
  filename(),
  on_menukey_script(),
  sectors(),
  stats(),
  target_time(),
  tileset("images/tiles.strf")
{
  _current = this;
}

Level::~Level()
{
  sectors.clear();
}

void
Level::save(const std::string& filepath, bool retry)
{
  //FIXME: It tests for directory in supertux/data, but saves into .supertux2.

  try {

    { // make sure the level directory exists
      std::string dirname = FileSystem::dirname(filepath);
      if(!PHYSFS_exists(dirname.c_str()))
      {
        if(!PHYSFS_mkdir(dirname.c_str()))
        {
          std::ostringstream msg;
          msg << "Couldn't create directory for level '"
              << dirname << "': " <<PHYSFS_getLastError();
          throw std::runtime_error(msg.str());
        }
      }

      PHYSFS_Stat statbuf;
      PHYSFS_stat(dirname.c_str(), &statbuf);
      if(statbuf.filetype != PHYSFS_FILETYPE_DIRECTORY)
      {
        std::ostringstream msg;
        msg << "Level path '" << dirname << "' is not a directory";
        throw std::runtime_error(msg.str());
      }
    }

    Writer writer(filepath);
    writer.start_list("supertux-level");
    // Starts writing to supertux level file. Keep this at the very beginning.

    writer.write("version", 2);
    writer.write("name", name, true);
    writer.write("author", author, false);
    writer.write("tileset", tileset, false);
    if (contact != "") {
      writer.write("contact", contact, false);
    }
    if (license != "") {
      writer.write("license", license, false);
    }
    if (on_menukey_script != "") {
      writer.write("on-menukey-script", on_menukey_script, false);
    }
    if (target_time){
      writer.write("target-time", target_time);
    }

    for(auto& sector : sectors) {
      sector->save(writer);
    }

    // Ends writing to supertux level file. Keep this at the very end.
    writer.end_list("supertux-level");
    log_warning << "Level saved as " << filepath << "." << std::endl;
  } catch(std::exception& e) {
    if (retry) {
      std::stringstream msg;
      msg << "Problem when saving level '" << filepath << "': " << e.what();
      throw std::runtime_error(msg.str());
    } else {
      log_warning << "Failed to save the level, retrying..." << std::endl;
      { // create the level directory again
        std::string dirname = FileSystem::dirname(filepath);
        if(!PHYSFS_mkdir(dirname.c_str()))
        {
          std::ostringstream msg;
          msg << "Couldn't create directory for level '"
              << dirname << "': " <<PHYSFS_getLastError();
          throw std::runtime_error(msg.str());
        }
      }
      save(filepath, true);
    }
  }
}

void
Level::add_sector(std::unique_ptr<Sector> sector)
{
  Sector* test = get_sector(sector->get_name());
  if (test != nullptr) {
    throw std::runtime_error("Trying to add 2 sectors with same name");
  } else {
    sectors.push_back(std::move(sector));
  }
}

Sector*
Level::get_sector(const std::string& name_) const
{
  for(auto const& sector : sectors) {
    if(sector->get_name() == name_) {
      return sector.get();
    }
  }
  return nullptr;
}

size_t
Level::get_sector_count() const
{
  return sectors.size();
}

Sector*
Level::get_sector(size_t num) const
{
  return sectors.at(num).get();
}

int
Level::get_total_coins() const
{
  int total_coins = 0;
  for(auto const& sector : sectors) {
    for(const auto& o: sector->gameobjects) {
      auto coin = dynamic_cast<Coin*>(o.get());
      if(coin)
      {
        total_coins++;
        continue;
      }
      auto block = dynamic_cast<BonusBlock*>(o.get());
      if(block)
      {
        if (block->contents == BonusBlock::CONTENT_COIN)
        {
          total_coins += block->hit_counter;
          continue;
        } else if (block->contents == BonusBlock::CONTENT_RAIN) {
          total_coins += 10;
          continue;
        } else if (block->contents == BonusBlock::CONTENT_EXPLODE) {
          total_coins += 10;
          continue;
        }
      }
      auto goldbomb = dynamic_cast<GoldBomb*>(o.get());
      if(goldbomb)
        total_coins += 10;
    }
  }
  return total_coins;
}

int
Level::get_total_badguys() const
{
  int total_badguys = 0;
  for(auto const& sector : sectors) {
    total_badguys += sector->get_total_badguys();
  }
  return total_badguys;
}

int
Level::get_total_secrets() const
{
  int total_secrets = 0;
  for(auto const& sector : sectors) {
    total_secrets += sector->get_total_count<SecretAreaTrigger>();
  }
  return total_secrets;
}

void
Level::reactivate()
{
  _current = this;
}

/* EOF */
