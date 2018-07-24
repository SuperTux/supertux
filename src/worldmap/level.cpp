//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmail.com>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "worldmap/level.hpp"

#include <physfs.h>

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "worldmap/worldmap.hpp"

namespace worldmap {

LevelTile::LevelTile(const std::string& basedir_, const ReaderMapping& lisp) :
  pos(),
  title(),
  solved(false),
  perfect(false),
  auto_play(false),
  sprite(),
  statistics(),
  target_time(),
  extro_script(),
  title_color(WorldMap::level_title_color),
  basedir(basedir_)
{
  lisp.get("name", name);
  lisp.get("x", pos.x);
  lisp.get("y", pos.y);
  lisp.get("auto-play", auto_play);

  std::string spritefile = "images/worldmap/common/leveldot.sprite";
  lisp.get("sprite", spritefile);
  sprite = SpriteManager::current()->create(spritefile);

  lisp.get("extro-script", extro_script);

  std::vector<float> vColor;
  if (lisp.get("color", vColor)) {
    title_color = Color(vColor);
  }

  if(basedir == "./")
    basedir = "";

  if (!PHYSFS_exists((basedir + name).c_str()))
  {
    log_warning << "level file '" << name
                << "' does not exist and will not be added to the worldmap" << std::endl;
    return;
  }
}

LevelTile::~LevelTile()
{
}

void
LevelTile::draw(DrawingContext& context)
{
  sprite->draw(context.color(), pos*32 + Vector(16, 16), LAYER_OBJECTS - 1);
}

void
LevelTile::update(float )
{
}

void
LevelTile::update_sprite_action()
{
  if(!solved)
    sprite->set_action("default");
  else
    sprite->set_action((sprite->has_action("perfect") && perfect) ? "perfect" : "solved");
}

void
LevelTile::set_solved(bool v)
{
  solved = v;
  update_sprite_action();
}

void
LevelTile::set_perfect(bool v)
{
  perfect = v;
  update_sprite_action();
}

} // namespace worldmap

/* EOF */
