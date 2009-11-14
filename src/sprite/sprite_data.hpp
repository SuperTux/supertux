//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef SUPERTUX_SPRITE_DATA_H
#define SUPERTUX_SPRITE_DATA_H

#include <string>
#include <vector>
#include <map>

#include "lisp/lisp.hpp"
#include "video/surface.hpp"

class SpriteData
{
public:
  /** cur has to be a pointer to data in the form of ((hitbox 5 10 0 0) ...) */
  SpriteData(const lisp::Lisp* cur, const std::string& basedir);
  ~SpriteData();

  const std::string& get_name() const
  {
    return name;
  }

private:
  friend class Sprite;

  struct Action
  {
    Action();
    ~Action();

    std::string name;

    /** Position correction */
    float x_offset;
    float y_offset;

    /** Hitbox width */
    float hitbox_w;

    /** Hitbox height */
    float hitbox_h;

    /** Drawing priority in queue */
    int z_order;

    /** Frames per second */
    float fps;

    std::vector<Surface*> surfaces;
  };

  typedef std::map <std::string, Action*> Actions;
  Actions actions;

  void parse_action(const lisp::Lisp* lispreader, const std::string& basedir);
  /** Get an action */
  Action* get_action(std::string act);

  std::string name;
};

#endif
