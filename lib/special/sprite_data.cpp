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

#include "sprite_data.h"
#include "app/globals.h"
#include "app/setup.h"
#include "video/drawing_context.h"

namespace SuperTux
{

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

SpriteData::SpriteData(lisp_object_t* cur)
{
  for(; !lisp_nil_p(cur); cur = lisp_cdr(cur)) {
    std::string token = lisp_symbol(lisp_car(lisp_car(cur)));
    lisp_object_t* data = lisp_car(lisp_cdr(lisp_car(cur)));
    LispReader reader(lisp_cdr(lisp_car(cur)));

    if(token == "name")
      name = lisp_string(data);
    else if(token == "action")
      parse_action(reader);
    else
      std::cerr << "Warning: Unknown sprite field: " << token << std::endl;
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
SpriteData::parse_action(LispReader& lispreader)
{
  Action* action = new Action;

  if(!lispreader.read_string("name", action->name)) {
    if(!actions.empty())
      throw std::runtime_error(
          "If there are more than one action, they need names!");
  }
  lispreader.read_int("x-offset", action->x_offset);
  lispreader.read_int("y-offset", action->y_offset);
  lispreader.read_int("z-order", action->z_order);
  lispreader.read_float("fps",     action->fps);

  /* TODO: add a top filter entry */
  std::vector <int> mask_color;
  lispreader.read_int_vector("apply-mask", mask_color);
  if(mask_color.size() == 4) {
    for(std::vector<Surface*>::iterator i = action->surfaces.begin();
        i < action->surfaces.end(); i++) {
      (*i)->apply_filter(MASK_FILTER, Color(mask_color));
    }
  }

  std::string mirror_action;
  lispreader.read_string("mirror-action", mirror_action);
  if(!mirror_action.empty()) {
    Action* act_tmp = get_action(mirror_action);
    if(act_tmp == NULL) {
      throw std::runtime_error("Could not mirror action. Action not found\n"
                   "Mirror actions must be defined after the real one!");
    } else {
      for(int i = 0; static_cast<unsigned int>(i) < act_tmp->surfaces.size();
          i++) {
        Surface* surface = new Surface(sdl_surface_from_sdl_surface(
              act_tmp->surfaces[i]->impl->get_sdl_surface(), true), true);
        surface->apply_filter(HORIZONTAL_FLIP_FILTER);
        action->surfaces.push_back(surface);
      }
    }
  } else { // Load images
    std::vector<std::string> images;
    if(!lispreader.read_string_vector("images", images)) {
      std::stringstream msg;
      msg << "Sprite '" << name << "' contains no images in action '"
          << action->name << "'.";
      throw std::runtime_error(msg.str());
    }

    for(std::vector<std::string>::size_type i = 0; i < images.size(); i++) {
      action->surfaces.push_back(
          new Surface(datadir + "/images/" + images[i], true));
    }
  }
  actions[action->name] = action;
}

SpriteData::Action*
SpriteData::get_action(std::string act)
{
  Actions::iterator i = actions.find(act);
  if(i == actions.end()) {
    std::cerr << "Warning: Action '" << act <<
      "' not found on Sprite '" << name << "'\n";
    return 0;
  }
  return i->second;
}

}
