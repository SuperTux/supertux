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

#include "sprite/sprite_data.hpp"

#include <algorithm>
#include <stdexcept>
#include <sstream>

#include "util/log.hpp"
#include "util/reader_mapping.hpp"

SpriteData::Action::Action() :
  name(),
  x_offset(0),
  y_offset(0),
  hitbox_w(0),
  hitbox_h(0),
  z_order(0),
  fps(10),
  loops(-1),
  has_custom_loops(false),
  surfaces()
{
}

SpriteData::Action::~Action()
{
}

SpriteData::SpriteData(const ReaderMapping& lisp, const std::string& basedir) :
  actions(),
  name()
{
  auto iter = lisp.get_iter();
  while(iter.next()) {
    if(iter.get_key() == "name") {
      iter.get(name);
    } else if(iter.get_key() == "action") {
      parse_action(iter.as_mapping(), basedir);
    } else {
      log_warning << "Unknown sprite field: " << iter.get_key() << std::endl;
    }
  }
  if(actions.empty())
    throw std::runtime_error("Error: Sprite without actions.");
}

SpriteData::~SpriteData()
{
}

void
SpriteData::parse_action(const ReaderMapping& lisp, const std::string& basedir)
{
  auto action = std::unique_ptr<Action>(new Action);

  if(!lisp.get("name", action->name)) {
    if(!actions.empty())
      throw std::runtime_error(
        "If there are more than one action, they need names!");
  }

  std::vector<float> hitbox;
  if (lisp.get("hitbox", hitbox)) {
    switch(hitbox.size()) {
      case 4:
        action->hitbox_h = hitbox[3];
        action->hitbox_w = hitbox[2];

        //fall-through
      case 2:
        action->y_offset = hitbox[1];
        action->x_offset = hitbox[0];
        break;

      default:
        throw std::runtime_error("hitbox should specify 2/4 coordinates");
    }
  }
  lisp.get("z-order", action->z_order);
  lisp.get("fps", action->fps);
  if(lisp.get("loops", action->loops))
  {
    action->has_custom_loops = true;
  }

  std::string mirror_action;
  if (lisp.get("mirror-action", mirror_action)) {
    const auto act_tmp = get_action(mirror_action);
    if(act_tmp == NULL) {
      std::ostringstream msg;
      msg << "Could not mirror action. Action not found: \"" << mirror_action << "\"\n"
          << "Mirror actions must be defined after the real one!";
      throw std::runtime_error(msg.str());
    } else {
      float max_w = 0;
      float max_h = 0;
      for(const auto& surf : act_tmp->surfaces) {
        auto surface = surf->clone();
        surface->hflip();
        max_w = std::max(max_w, (float) surface->get_width());
        max_h = std::max(max_h, (float) surface->get_height());
        action->surfaces.push_back(surface);
      }
      if (action->hitbox_w < 1) action->hitbox_w = max_w - action->x_offset;
      if (action->hitbox_h < 1) action->hitbox_h = max_h - action->y_offset;
    }
  } else { // Load images
    std::vector<std::string> images;
    if(!lisp.get("images", images)) {
      std::stringstream msg;
      msg << "Sprite '" << name << "' contains no images in action '"
          << action->name << "'.";
      throw std::runtime_error(msg.str());
    } else {
      float max_w = 0;
      float max_h = 0;
      for(const auto& image : images) {
        auto surface = Surface::create(basedir + image);
        max_w = std::max(max_w, (float) surface->get_width());
        max_h = std::max(max_h, (float) surface->get_height());
        action->surfaces.push_back(surface);
      }
      if (action->hitbox_w < 1) action->hitbox_w = max_w - action->x_offset;
      if (action->hitbox_h < 1) action->hitbox_h = max_h - action->y_offset;
    }
  }
  actions[action->name] = std::move(action);
}

const SpriteData::Action*
SpriteData::get_action(const std::string& act) const
{
  Actions::const_iterator i = actions.find(act);
  if(i == actions.end()) {
    return nullptr;
  }
  return i->second.get();
}

/* EOF */
