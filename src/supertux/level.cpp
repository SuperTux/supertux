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
#include "supertux/sector.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/tile_set.hpp"
#include "trigger/secretarea_trigger.hpp"
#include "util/reader.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

#include <sstream>
#include <stdexcept>

using namespace std;

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
  tileset(NULL),
  free_tileset(false)
{
}

Level::~Level()
{
  for(Sectors::iterator i = sectors.begin(); i != sectors.end(); ++i)
    delete *i;
  if(free_tileset)
    delete tileset;
}

void
Level::load(const std::string& filepath)
{
  try {
    filename = filepath;
    register_translation_directory(filepath);
    auto doc = ReaderDocument::parse(filepath);
    auto root = doc.get_root();

    if(root.get_name() != "supertux-level")
      throw std::runtime_error("file is not a supertux-level file.");

    auto level = root.get_mapping();

    int version = 1;
    level.get("version", version);
    if(version == 1) {
      log_info << "[" <<  filepath << "] level uses old format: version 1" << std::endl;
      tileset = TileManager::current()->get_tileset("images/tiles.strf");
      load_old_format(level);
      return;
    }

    ReaderCollection tilesets_lisp;
    if (level.get("tilesets", tilesets_lisp)) {
      tileset      = TileManager::current()->parse_tileset_definition(tilesets_lisp).release();
      free_tileset = true;
    }

    std::string tileset_name;
    if(level.get("tileset", tileset_name)) {
      if(tileset != NULL) {
        log_warning << "[" <<  filepath << "] multiple tilesets specified in level" << std::endl;
      } else {
        tileset = TileManager::current()->get_tileset(tileset_name);
      }
    }
    /* load default tileset */
    if(tileset == NULL) {
      tileset = TileManager::current()->get_tileset("images/tiles.strf");
    }
    current_tileset = tileset;

    contact = "";
    license = "";

    if (level.get("version", version))
    {
      if(version > 2) {
        log_warning << "[" <<  filepath << "] level format newer than application" << std::endl;
      }
    }

    level.get("name", name);
    level.get("author", author);
    level.get("contact", contact);
    level.get("license", license);
    level.get("on-menukey-script", on_menukey_script);
    level.get("target-time", target_time);

    auto iter = level.get_iter();
    while(iter.next()) {
      if (iter.get_key() == "sector") {
        Sector* sector = new Sector(this);
        sector->parse(iter.as_mapping());
        add_sector(sector);
      }
    }

    if (license.empty()) {
      log_warning << "[" <<  filepath << "] The level author \"" << author << "\" did not specify a license for this level \"" << name << "\". You might not be allowed to share it." << std::endl;
    }
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Problem when reading level '" << filepath << "': " << e.what();
    throw std::runtime_error(msg.str());
  }

  current_tileset = NULL;
}

void
Level::load_old_format(const ReaderMapping& reader)
{
  reader.get("name", name);
  reader.get("author", author);

  Sector* sector = new Sector(this);
  sector->parse_old_format(reader);
  add_sector(sector);
}

void
Level::add_sector(Sector* sector)
{
  Sector* test = get_sector(sector->get_name());
  if(test != 0) {
    throw std::runtime_error("Trying to add 2 sectors with same name");
  }
  sectors.push_back(sector);
}

Sector*
Level::get_sector(const std::string& name_) const
{
  for(Sectors::const_iterator i = sectors.begin(); i != sectors.end(); ++i) {
    Sector* sector = *i;
    if(sector->get_name() == name_)
      return sector;
  }

  return 0;
}

size_t
Level::get_sector_count() const
{
  return sectors.size();
}

Sector*
Level::get_sector(size_t num) const
{
  return sectors.at(num);
}

int
Level::get_total_coins() const
{
  int total_coins = 0;
  for(Sectors::const_iterator i = sectors.begin(); i != sectors.end(); ++i) {
    Sector* sector = *i;
    for(auto o = sector->gameobjects.begin(); o != sector->gameobjects.end(); ++o) {
      Coin* coin = dynamic_cast<Coin*>(o->get());
      if(coin)
      {
        total_coins++;
        continue;
      }
      BonusBlock *block = dynamic_cast<BonusBlock*>(o->get());
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
      GoldBomb *goldbomb = dynamic_cast<GoldBomb*>(o->get());
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
  for(Sectors::const_iterator i = sectors.begin(); i != sectors.end(); ++i)
    total_badguys += (*i)->get_total_badguys();
  return total_badguys;
}

int
Level::get_total_secrets() const
{
  int total_secrets = 0;
  for(auto i = sectors.begin(); i != sectors.end(); ++i)
    total_secrets += (*i)->get_total_count<SecretAreaTrigger>();
  return total_secrets;
}

/* EOF */
