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

#include <config.h>

#include <iostream>
#include <cmath>
#include <sstream>
#include <stdexcept>

#include "sprite_data.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"
#include "lisp/list_iterator.hpp"
#include "util/log.hpp"

SpriteData::Action::Action()
{
  x_offset = 0;
  y_offset = 0;
  hitbox_w = 0;
  hitbox_h = 0;
  z_order = 0;
  fps = 10;
}

SpriteData::Action::~Action()
{
  for(std::vector<Surface*>::iterator i = surfaces.begin();
      i != surfaces.end(); ++i)
    delete *i;
}

SpriteData::SpriteData(const lisp::Lisp* lisp, const std::string& basedir)
{
  lisp::ListIterator iter(lisp);
  while(iter.next()) {
    if(iter.item() == "name") {
      iter.value()->get(name);
    } else if(iter.item() == "action") {
      parse_action(iter.lisp(), basedir);
    } else {
      log_warning << "Unknown sprite field: " << iter.item() << std::endl;
    }
  }
  if(actions.empty())
    throw std::runtime_error("Error: Sprite without actions.");
}

SpriteData::~SpriteData()
{
  for(Actions::iterator i=actions.begin(); i != actions.end(); ++i)
    delete i->second;
}

void
SpriteData::parse_action(const lisp::Lisp* lisp, const std::string& basedir)
{
  Action* action = new Action;

  if(!lisp->get("name", action->name)) {
    if(!actions.empty())
      throw std::runtime_error(
          "If there are more than one action, they need names!");
  }
  std::vector<float> hitbox;
  if (lisp->get("hitbox", hitbox)) {
    if (hitbox.size() != 4) throw std::runtime_error("hitbox must specify exactly 4 coordinates");
    action->x_offset = hitbox[0];
    action->y_offset = hitbox[1];
    action->hitbox_w = hitbox[2];
    action->hitbox_h = hitbox[3];
  }
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
      float max_w = 0;
      float max_h = 0;
      for(int i = 0; static_cast<unsigned int>(i) < act_tmp->surfaces.size();
          i++) {
        Surface* surface = new Surface(*(act_tmp->surfaces[i]));
        surface->hflip();
        max_w = std::max(max_w, (float) surface->get_width());
        max_h = std::max(max_h, (float) surface->get_height());
        action->surfaces.push_back(surface);
      }
      if (action->hitbox_w < 1) action->hitbox_w = max_w;
      if (action->hitbox_h < 1) action->hitbox_h = max_h;
    }
  } else { // Load images
    std::vector<std::string> images;
    if(!lisp->get("images", images)) {
      std::stringstream msg;
      msg << "Sprite '" << name << "' contains no images in action '"
          << action->name << "'.";
      throw std::runtime_error(msg.str());
    }

    float max_w = 0;
    float max_h = 0;
    for(std::vector<std::string>::size_type i = 0; i < images.size(); i++) {
      Surface* surface = new Surface(basedir + images[i]);
      max_w = std::max(max_w, (float) surface->get_width());
      max_h = std::max(max_h, (float) surface->get_height());
      action->surfaces.push_back(surface);
    }
    if (action->hitbox_w < 1) action->hitbox_w = max_w;
    if (action->hitbox_h < 1) action->hitbox_h = max_h;
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
