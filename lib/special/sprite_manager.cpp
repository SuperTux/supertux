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

#include "sprite_manager.h"
#include "sprite_data.h"
#include "sprite.h"
#include "lisp/lisp.h"
#include "lisp/parser.h"
#include "lisp/list_iterator.h"

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
  lisp::Parser parser;
  try {
    std::auto_ptr<lisp::Lisp> root (parser.parse(filename));

    const lisp::Lisp* resources = root->get_lisp("supertux-resources");
    if(!resources)
      throw std::runtime_error("file is not a supertux-resources files");

    lisp::ListIterator iter(resources);
    while(iter.next()) {
      if(iter.item() == "sprite") {
        SpriteData* spritedata = new SpriteData(iter.lisp());

        printf("Spr: %s.\n", spritedata->get_name().c_str());
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
        std::cout << "SpriteManager: Unknown tag '" << iter.item() 
          << "' in spritefile.\n";
      }
    }
  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Couldn't load file '" << filename << "': " << e.what() << "\n";
    throw std::runtime_error(msg.str());
  }
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

