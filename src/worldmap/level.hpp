//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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
#ifndef __LEVEL_TILE_HPP__
#define __LEVEL_TILE_HPP__

#include <memory>
#include <string>
#include "math/vector.hpp"
#include "game_object.hpp"
#include "statistics.hpp"
#include "video/surface.hpp"

class Sprite;

namespace WorldMapNS
{

class LevelTile : public GameObject
{
public:
  LevelTile(const std::string& basedir, const lisp::Lisp* lisp);
  virtual ~LevelTile();

  virtual void draw(DrawingContext& context);
  virtual void update(float elapsed_time);

  Vector pos;
  std::string title;
  bool solved;
  bool auto_play; /**< true if Tux should automatically enter this level if it's unfinished */

  std::auto_ptr<Sprite> sprite;

  /** Statistics for level tiles */
  Statistics statistics;

  /** Script that is run when the level is successfully finished */
  std::string extro_script;

  /** return Surface of level picture or 0 if no picture is available */
  const Surface* get_picture();

private:
  std::string basedir;
  bool picture_cached;
  Surface* picture;

};

}

#endif
