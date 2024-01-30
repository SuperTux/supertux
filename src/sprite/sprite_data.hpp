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

#ifndef HEADER_SUPERTUX_SPRITE_SPRITE_DATA_HPP
#define HEADER_SUPERTUX_SPRITE_SPRITE_DATA_HPP

#include <map>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

#include "video/color.hpp"
#include "video/surface_ptr.hpp"

class ReaderMapping;

class SpriteData final
{
  friend class Sprite;

public:
  /**
   * Sprite from data.
   * `mapping` has to be a pointer to data in the form of "((hitbox 5 10 0 0) ...)".
   */
  SpriteData(const ReaderMapping& mapping);
  /** Single-image sprite */
  SpriteData(const std::string& image);
  /** Dummy texture sprite */
  SpriteData();

  const std::string& get_name() const
  {
    return name;
  }

private:
  struct LinkedLightSprite final
  {
    LinkedLightSprite(const std::string& file_) :
      file(file_), color()
    {}

    std::string file;
    Color color;
  };
  typedef std::unordered_map<std::string, std::string> LinkedSprites;

  struct Action
  {
    Action();

    std::string name;

    /** Position correction */
    float x_offset;
    float y_offset;
    float flip_offset;

    /** Hitbox width */
    float hitbox_w;

    /** Hitbox height */
    float hitbox_h;

    /** Whether the hitbox is unisolid */
    bool hitbox_unisolid;

    /** Frames per second */
    float fps;

    /** Loops (-1 = looping endlessly) */
    int loops;

    /** Starting frame for animation loops */
    int loop_frame;

    /** Flag that gets set to true if the action
        has custom loops defined */
    bool has_custom_loops;

    /** A unique identifier that's shared across
        actions that should continue each other
        (aka not reset when switching from one
        to another) */
    std::string family_name;

    std::vector<SurfacePtr> surfaces;

    std::optional<LinkedLightSprite> linked_light_sprite;
    LinkedSprites linked_sprites;
  };

  typedef std::map<std::string, std::unique_ptr<Action> > Actions;

  static std::unique_ptr<Action> create_action_from_surface(SurfacePtr surface);

  void parse_action(const ReaderMapping& mapping);
  /** Get an action */
  const Action* get_action(const std::string& act) const;

private:
  Actions actions;
  std::string name;

  std::optional<LinkedLightSprite> linked_light_sprite;
  LinkedSprites linked_sprites;
};

#endif

/* EOF */
