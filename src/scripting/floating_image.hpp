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

#ifndef HEADER_SUPERTUX_SCRIPTING_FLOATING_IMAGE_HPP
#define HEADER_SUPERTUX_SCRIPTING_FLOATING_IMAGE_HPP

#ifndef SCRIPTING_API
#include <memory>

class FloatingImage;

#include "scripting/game_object.hpp"
#endif

namespace scripting {

class FloatingImage final
#ifndef SCRIPTING_API
  : public GameObject<::FloatingImage>
#endif
{
public:
  FloatingImage(const std::string& spritefile);

  /**
   * Sets the layer of the floating image
   * @param layer Target layer
   */
  void set_layer(int layer);
  /**
   * Returns the layer the floating image is on
   */
  int get_layer() const;
  /**
   * Sets the location of the image, in relation to the current anchor point
   * @param x X coordinate
   * @param y Y coordinate
   */
  void set_pos(float x, float y);
  /**
   * Returns the image's X coordinate relative to the current anchor point
   */
  float get_pos_x() const;
  /**
   * Returns the image's Y coordinate relative to the current anchor point
   */
  float get_pos_y() const;
  /**
   * Sets the image's anchor point
   * @param anchor Anchor point as represented by the ANCHOR_* constants
   */
  void set_anchor_point(int anchor);
  /**
   * Returns the current anchor point
   */
  int get_anchor_point() const;
  /**
   * Sets the visibility of the floating image
   * @param visible Visibility
   */
  void set_visible(bool visible);
  /**
   * Returns the visibility state of the floating image
   */
  bool get_visible() const;
  /**
   * Sets the action of the image
   * This is only useful when the image is a sprite
   * @param action Name of the action, as defined in the sprite
   */
  void set_action(const std::string& action);
  /**
   * Returns the action of the image
   * This is only useful when the image is a sprite
   * @return Name of the action, as defined in the sprite
   */
  std::string get_action() const;
  void fade_in(float fadetime);
  void fade_out(float fadetime);

#ifndef SCRIPTING_API
private:
  FloatingImage(const FloatingImage&) = delete;
  FloatingImage& operator=(const FloatingImage&) = delete;
#endif
};

} // namespace scripting

#endif

/* EOF */
