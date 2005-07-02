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
#include <cmath>
#include <sstream>
#include <stdexcept>

#include "sprite_data.hpp"
#include "resources.hpp"
#include "video/drawing_context.hpp"
#include "lisp/list_iterator.hpp"

SpriteData::Action::Action()
{
  x_offset = 0;
  y_offset = 0;
  z_order = 0;   
  fps = 10;
}

SpriteData::Action::~Action()
{
  for(std::vector<Surface*>::iterator i = surfaces.begin();
      i != surfaces.end(); ++i)
    delete *i;
}

SpriteData::SpriteData(const lisp::Lisp* lisp)
{
  lisp::ListIterator iter(lisp);
  while(iter.next()) {
    if(iter.item() == "name") {
      iter.value()->get(name);
    } else if(iter.item() == "action") {
      parse_action(iter.lisp());
    } else {
      std::cerr << "Unknown sprite field: " << iter.item() << "\n";
    }
  }
  if(name.empty())
    throw std::runtime_error("Error: Sprite wihtout name.");
  if(actions.empty())
    throw std::runtime_error("Error: Sprite wihtout actions.");
}

SpriteData::~SpriteData()
{
  for(Actions::iterator i=actions.begin(); i != actions.end(); ++i)
    delete i->second;
}

void
SpriteData::parse_action(const lisp::Lisp* lisp)
{
  Action* action = new Action;

  if(!lisp->get("name", action->name)) {
    if(!actions.empty())
      throw std::runtime_error(
          "If there are more than one action, they need names!");
  }
  lisp->get("x-offset", action->x_offset);
  lisp->get("y-offset", action->y_offset);
  lisp->get("z-order", action->z_order);
  lisp->get("fps", action->fps);

  std::string mirror_action;
  lisp->get("mirror-action", mirror_action);
  if(!mirror_action.empty()) {
    Action* act_tmp = get_action(mirror_action);
    if(act_tmp == NULL) {
      throw std::runtime_error("Could not mirror action. Action not found\n"
                   "Mirror actions must be defined after the real one!");
    } else {
      for(int i = 0; static_cast<unsigned int>(i) < act_tmp->surfaces.size();
          i++) {
        Surface* surface = new Surface(sdl_surface_from_sdl_surface(
              act_tmp->surfaces[i]->impl->get_sdl_surface()), true);
        surface->apply_filter(HORIZONTAL_FLIP_FILTER);
        action->surfaces.push_back(surface);
      }
    }
  } else { // Load images
    std::vector<std::string> images;
    if(!lisp->get_vector("images", images)) {
      std::stringstream msg;
      msg << "Sprite '" << name << "' contains no images in action '"
          << action->name << "'.";
      throw std::runtime_error(msg.str());
    }

    for(std::vector<std::string>::size_type i = 0; i < images.size(); i++) {
      action->surfaces.push_back(new Surface("images/" + images[i], true));
    }
  }
  actions[action->name] = action;
}

SpriteData::Action*
SpriteData::get_action(std::string act)
{
  Actions::iterator i = actions.find(act);
  if(i == actions.end()) {
    return 0;
  }
  return i->second;
}

