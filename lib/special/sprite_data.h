//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>,
//            (C) 2004 Matthias Braun <matze@braunis.de>
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

#include "lisp/lisp.h"
#include "video/surface.h"

namespace SuperTux
{

  class SpriteData
  {
  public:
    /** cur has to be a pointer to data in the form of ((x-offset 5)
      (y-offset 10) ...) */
    SpriteData(const lisp::Lisp* cur);                                         
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
      int x_offset;
      int y_offset;
      /** Drawing priority in queue */
      int z_order;

      /** Frames per second */
      float fps;

      /** Mirror is used to avoid duplicating left and right side
        sprites */
      // bool mirror;

      std::vector<Surface*> surfaces;
    };

    typedef std::map <std::string, Action*> Actions;
    Actions actions;

    void parse_action(const lisp::Lisp* lispreader);
    /** Get an action */
    Action* get_action(std::string act);

    std::string name;
  };
}

#endif

