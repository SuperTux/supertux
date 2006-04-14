//  $Id: worldmap.hpp 3327 2006-04-13 15:02:40Z ravu_al_hemio $
//
//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#ifndef __WORLDMAP_LEVEL_HPP__
#define __WORLDMAP_LEVEL_HPP__

#include <memory>
#include <string>
#include "math/vector.hpp"
#include "game_object.hpp"
#include "statistics.hpp"

class Sprite;

namespace WorldMapNS
{

class Level : public GameObject
{
public:
  Level(const std::string& basedir, const lisp::Lisp* lisp);
  virtual ~Level();

  virtual void draw(DrawingContext& context);
  virtual void update(float elapsed_time);
  
  Vector pos;
  std::string name;
  std::string title;
  bool solved;

  std::auto_ptr<Sprite> sprite;

  /** Statistics for level tiles */
  Statistics statistics;

  /** Script that is run when the level is successfully finished */
  std::string extro_script;

  /** If false, disables the auto walking after finishing a level */
  bool auto_path;
};

}

#endif

