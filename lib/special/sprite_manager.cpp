//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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
#include <config.h>

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "utils/lispreader.h"
#include "sprite_manager.h"
#include "sprite_data.h"
#include "sprite.h"

namespace SuperTux
{

SpriteManager::SpriteManager(const std::string& filename)
{
  load_resfile(filename);
}

SpriteManager::~SpriteManager()
{
  for(Sprites::iterator i = sprites.begin(); i != sprites.end(); ++i) {
    delete i->second;
  }
}

void
SpriteManager::load_resfile(const std::string& filename)
{
  lisp_object_t* root_obj = lisp_read_from_file(filename);
  if (!root_obj)
    {
      std::cout << "SpriteManager: Couldn't load: " << filename << std::endl;
      return;
    }

  lisp_object_t* cur = root_obj;

  if (strcmp(lisp_symbol(lisp_car(cur)), "supertux-resources") != 0)
    return;
  cur = lisp_cdr(cur);

  while(cur) {
    lisp_object_t* el = lisp_car(cur);

    if (strcmp(lisp_symbol(lisp_car(el)), "sprite") == 0) {
      SpriteData* spritedata = new SpriteData(lisp_cdr(el));

      Sprites::iterator i = sprites.find(spritedata->get_name());
      if (i == sprites.end()) {
        sprites[spritedata->get_name()] = spritedata;
      } else {
        delete i->second;
        i->second = spritedata;
        std::cout << "Warning: dulpicate entry: '" << spritedata->get_name()
          << "' in spritefile." << std::endl;
      }
    } else {
      std::cout << "SpriteManager: Unknown tag in spritefile.\n";
    }

    cur = lisp_cdr(cur);
  }

  lisp_free(root_obj);
}

Sprite*
SpriteManager::create(const std::string& name)
{
  Sprites::iterator i = sprites.find(name);
  if(i == sprites.end()) {
    std::stringstream msg;
    msg << "Sprite '" << name << "' not found.";
    throw std::runtime_error(msg.str());
  }
  return new Sprite(*i->second);
}

}

