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

#include <physfs.h>
#include <sexp/io.hpp>
#include <sexp/value.hpp>

#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/reader_object.hpp"
#include "video/surface.hpp"
#include "video/texture_manager.hpp"

SpriteData::Action::Action() :
  name(),
  x_offset(0),
  y_offset(0),
  flip_offset(0),
  hitbox_w(0),
  hitbox_h(0),
  hitbox_unisolid(false),
  fps(10),
  loops(-1),
  loop_frame(1),
  has_custom_loops(false),
  family_name(),
  surfaces()
{
}

SpriteData::SpriteData(const ReaderMapping& mapping) :
  actions(),
  name(),
  linked_light_sprite(),
  linked_sprites()
{
  auto iter = mapping.get_iter();
  while (iter.next())
  {
    if (iter.get_key() == "name")
    {
      iter.get(name);
    }
    else if (iter.get_key() == "action")
    {
      parse_action(iter.as_mapping());
    }
    else if (iter.get_key() == "linked-sprites") // "(linked-sprites ({key} "{path}" {color r/g/b [optional]}) )"
    {
      auto iter_sprites = iter.as_mapping().get_iter();
      while (iter_sprites.next())
      {
        const auto& sx = iter_sprites.as_mapping().get_sexp();
        const auto& arr = sx.as_array();

        std::string filepath = FileSystem::join(mapping.get_doc().get_directory(), arr[1].as_string());
        if (!PHYSFS_exists(filepath.c_str())) // If file path is not relative to current directory, make it relative to root
          filepath = arr[1].as_string();

        if (arr[0].as_string() == "light") // The key "light" is reserved for light sprites
        {
          linked_light_sprite = LinkedLightSprite(filepath);
          if (arr.size() >= 5) // Color has been specified
          {
            linked_light_sprite->color = Color(arr[2].as_float(), arr[3].as_float(),
                                               arr[4].as_float());
          }
        }
        else
        {
          linked_sprites[arr[0].as_string()] = filepath;
        }
      }
    }
    else
    {
      log_warning << "Unknown sprite field: " << iter.get_key() << std::endl;
    }
  }
  if (actions.empty())
    throw std::runtime_error("Error: Sprite without actions.");
}

SpriteData::SpriteData(const std::string& image) :
  actions(),
  name(),
  linked_light_sprite(),
  linked_sprites()
{
  auto surface = Surface::from_file(image);
  if (!TextureManager::current()->last_load_successful())
    throw std::runtime_error("Cannot load image.");

  auto action = create_action_from_surface(surface);
  action->name = "default";
  actions[action->name] = std::move(action);
}

SpriteData::SpriteData() :
  actions(),
  name(),
  linked_light_sprite(),
  linked_sprites()
{
  auto surface = Surface::from_texture(TextureManager::current()->create_dummy_texture());
  auto action = create_action_from_surface(surface);
  action->name = "default";
  actions[action->name] = std::move(action);
}

std::unique_ptr<SpriteData::Action>
SpriteData::create_action_from_surface(SurfacePtr surface)
{
  auto action = std::make_unique<Action>();

  action->hitbox_w = static_cast<float>(surface->get_width());
  action->hitbox_h = static_cast<float>(surface->get_height());
  action->surfaces.push_back(surface);

  return action;
}

void
SpriteData::parse_action(const ReaderMapping& mapping)
{
  auto action = std::make_unique<Action>();

  if (!mapping.get("name", action->name))
  {
    if (!actions.empty())
      throw std::runtime_error("If there are more than one action, they need names!");
  }

  std::vector<float> hitbox;
  if (mapping.get("hitbox", hitbox))
  {
    switch (hitbox.size())
    {
      case 4:
        action->hitbox_h = hitbox[3];
        action->hitbox_w = hitbox[2];
        [[fallthrough]];
      case 2:
        action->y_offset = hitbox[1];
        action->x_offset = hitbox[0];
        break;

      default:
        throw std::runtime_error("hitbox should specify 2/4 coordinates");
    }
  }
  mapping.get("flip-offset", action->flip_offset);
  mapping.get("unisolid", action->hitbox_unisolid);
  mapping.get("fps", action->fps);
  if (mapping.get("loops", action->loops))
  {
    action->has_custom_loops = true;
  }
  if (mapping.get("loop-frame", action->loop_frame))
  {
    if (action->loop_frame < 1)
    {
      log_warning << "'loop-frame' of action '" << action->name << "' in sprite '" << name << "' set to a value below 1." << std::endl;
      action->loop_frame = 1;
    }
  }

  if (!mapping.get("family_name", action->family_name))
  {
    action->family_name = "::" + action->name;
  }

  std::string mirror_action;
  std::string flip_action;
  std::string clone_action;
  if (mapping.get("mirror-action", mirror_action))
  {
    const auto act_tmp = get_action(mirror_action);
    if (act_tmp == nullptr)
    {
      std::ostringstream msg;
      msg << "Could not mirror action. Action not found: \"" << mirror_action << "\"\n"
          << "Mirror actions must be defined after the real one!";
      throw std::runtime_error(msg.str());
    }
    else
    {
      float max_w = 0;
      float max_h = 0;
      for (const auto& surf : act_tmp->surfaces)
      {
        auto surface = surf->clone(HORIZONTAL_FLIP);
        max_w = std::max(max_w, static_cast<float>(surface->get_width()));
        max_h = std::max(max_h, static_cast<float>(surface->get_height()));
        action->surfaces.push_back(surface);
      }

      if (action->hitbox_w < 1 && action->hitbox_h < 1)
      {
        action->hitbox_w = act_tmp->hitbox_w;
        action->hitbox_h = act_tmp->hitbox_h;
        action->x_offset = act_tmp->x_offset;
        action->y_offset = act_tmp->y_offset;
      }

      if (!action->has_custom_loops && act_tmp->has_custom_loops)
      {
        action->has_custom_loops = act_tmp->has_custom_loops;
        action->loops = act_tmp->loops;
      }

      if (action->fps == 0)
      {
        action->fps = act_tmp->fps;
      }

      if (action->family_name == "::" + action->name)
      {
        action->family_name = act_tmp->family_name;
      }
    }
  }
  else if (mapping.get("flip-action", flip_action))
  {
    const auto act_tmp = get_action(flip_action);
    if (act_tmp == nullptr)
    {
      std::ostringstream msg;
      msg << "Could not flip action. Action not found: \"" << flip_action << "\"\n"
          << "Flip actions must be defined after the real one!";
      throw std::runtime_error(msg.str());
    }
    else
    {
      float max_w = 0;
      float max_h = 0;
      for (const auto& surf : act_tmp->surfaces)
      {
        auto surface = surf->clone(VERTICAL_FLIP);
        max_w = std::max(max_w, static_cast<float>(surface->get_width()));
        max_h = std::max(max_h, static_cast<float>(surface->get_height()));
        action->surfaces.push_back(surface);
      }

      if (action->hitbox_w < 1 && action->hitbox_h < 1)
      {
        action->hitbox_w = act_tmp->hitbox_w;
        action->hitbox_h = act_tmp->hitbox_h;
        action->x_offset = act_tmp->x_offset;
        action->y_offset = act_tmp->y_offset;
      }

      if (!action->has_custom_loops && act_tmp->has_custom_loops)
      {
        action->has_custom_loops = act_tmp->has_custom_loops;
        action->loops = act_tmp->loops;
      }

      if (action->fps == 0)
      {
        action->fps = act_tmp->fps;
      }

      if (action->family_name == "::" + action->name) {
        action->family_name = act_tmp->family_name;
      }
    }
  }
  else if (mapping.get("clone-action", clone_action))
  {
    const auto* act_tmp = get_action(clone_action);
    if (act_tmp == nullptr)
    {
      std::ostringstream msg;
      msg << "Could not clone action. Action not found: \"" << clone_action << "\"\n"
          << "Clone actions must be defined after the real one!";
      throw std::runtime_error(msg.str());
    }
    else
    {
      // copy everything except the name (Semphris: and the family name)
      const std::string oldname = action->name;
      const std::string oldfam = action->family_name;
      *action = *act_tmp;
      action->name = oldname;
      action->family_name = oldfam;

      if (action->family_name == "::" + action->name) {
        action->family_name = act_tmp->family_name;
      }
    }
  }
  else
  { // Load images
    std::optional<ReaderCollection> surfaces_collection;
    std::vector<std::string> images;
    std::optional<ReaderMapping> regions_mapping;
    if (mapping.get("regions", regions_mapping)) // Regions from images
    {
      float max_w = 0;
      float max_h = 0;

      auto iter = regions_mapping->get_iter();
      while (iter.next())
      {
        if (iter.get_key() != "region")
        {
          log_warning << "Unknown field '" << iter.get_key() << "' under 'regions'." << std::endl;
          continue;
        }

        const auto& sx = iter.as_mapping().get_sexp();
        const auto& arr = sx.as_array();
        if (arr.size() != 6)
        {
          log_warning << "(region IMAGE_FILE X Y WIDTH HEIGHT) tag malformed: " << sx << std::endl;
          continue;
        }

        Rect region;
        region.left = arr[2].as_int();
        region.top = arr[3].as_int();
        const int w = arr[4].as_int();
        const int h = arr[5].as_int();
        region.right = region.left + w;
        region.bottom = region.top + h;

        max_w = std::max(max_w, static_cast<float>(w));
        max_h = std::max(max_w, static_cast<float>(h));

        auto surface = Surface::from_file(FileSystem::join(mapping.get_doc().get_directory(),
                                                           arr[1].as_string()),
                                          region);
        action->surfaces.push_back(surface);
      }

      if (action->hitbox_w < 1) action->hitbox_w = max_w - action->x_offset;
      if (action->hitbox_h < 1) action->hitbox_h = max_h - action->y_offset;
    }
    else if (mapping.get("images", images))
    {
      float max_w = 0;
      float max_h = 0;
      for (const auto& image : images)
      {
        auto surface = Surface::from_file(FileSystem::join(mapping.get_doc().get_directory(), image));
        max_w = std::max(max_w, static_cast<float>(surface->get_width()));
        max_h = std::max(max_h, static_cast<float>(surface->get_height()));
        action->surfaces.push_back(surface);
      }
      if (action->hitbox_w < 1) action->hitbox_w = max_w - action->x_offset;
      if (action->hitbox_h < 1) action->hitbox_h = max_h - action->y_offset;
    }
    else if (mapping.get("surfaces", surfaces_collection))
    {
      for (const auto& i : surfaces_collection->get_objects())
      {
        if (i.get_name() == "surface")
        {
          action->surfaces.push_back(Surface::from_reader(i.get_mapping()));
        }
        else
        {
          std::stringstream msg;
          msg << "Sprite '" << name << "' unknown tag in 'surfaces' << " << i.get_name();
          throw std::runtime_error(msg.str());
        }
      }

      // calculate hitbox
      float max_w = 0;
      float max_h = 0;
      for (const auto& surface : action->surfaces)
      {
        max_w = std::max(max_w, static_cast<float>(surface->get_width()));
        max_h = std::max(max_h, static_cast<float>(surface->get_height()));
      }
      if (action->hitbox_w < 1) action->hitbox_w = max_w - action->x_offset;
      if (action->hitbox_h < 1) action->hitbox_h = max_h - action->y_offset;
    }
    else
    {
      std::stringstream msg;
      msg << "Sprite '" << name << "' contains no images in action '"
          << action->name << "'.";
      throw std::runtime_error(msg.str());
    }
  }

  // Reset loop-frame, if it's specified in current action, but not-in-range of total frames.
  const int frames = static_cast<int>(action->surfaces.size());
  if (action->loop_frame > frames && frames > 0)
  {
    log_warning << "'loop-frame' of action '" << action->name << "' in sprite '" << name << "' not-in-range of total frames." << std::endl;
    action->loop_frame = 1;
  }

  actions[action->name] = std::move(action);
}

const SpriteData::Action*
SpriteData::get_action(const std::string& act) const
{
  Actions::const_iterator i = actions.find(act);
  if (i == actions.end()) {
    return nullptr;
  }
  return i->second.get();
}

/* EOF */
