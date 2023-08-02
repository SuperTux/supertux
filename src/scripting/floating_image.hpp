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

/**
 * @summary This class provides the ability to create, edit, and remove images floating in midair on the screen, such as the SuperTux logo.
            It is implemented as a wrapper around a sprite, so any sprite actions are applicable.
 * @instances Floating Images are created in a script or from the console. Constructor:${SRG_NEWPARAGRAPH}
              """"""<floatimage> <- FloatingImage(string filename)""""""${SRG_NEWPARAGRAPH}
              This creates a ""FloatingImage"" from ""filename"" (which is relative to the data directory root).
 */
class FloatingImage final
#ifndef SCRIPTING_API
  : public GameObject<::FloatingImage>
#endif
{
public:
  FloatingImage(const std::string& spritefile);

  /**
   * Sets the layer of the floating image.
   * @param int $layer
   */
  void set_layer(int layer);
  /**
   * Returns the layer the floating image is on.
   */
  int get_layer() const;
  /**
   * Sets the location of the image in relation to the current anchor point.
   * @param float $x
   * @param float $y
   */
  void set_pos(float x, float y);
  /**
   * Returns the image's X coordinate relative to the current anchor point.
   */
  float get_pos_x() const;
  /**
   * Returns the image's Y coordinate relative to the current anchor point.
   */
  float get_pos_y() const;
  /**
   * Sets the image's anchor point.
   * @param int $anchor Anchor point as represented by the ""ANCHOR_*"" constants (see ${SRG_REF_AnchorPoints}).
   */
  void set_anchor_point(int anchor);
  /**
   * Returns the current anchor point.
   */
  int get_anchor_point() const;
  /**
   * Sets the visibility of the floating image.
   * @param bool $visible
   */
  void set_visible(bool visible);
  /**
   * Returns the visibility state of the floating image.
   */
  bool get_visible() const;
  /**
   * Sets the action of the image.
   * This is only useful when the image is a sprite.
   * @param string $action Name of the action, as defined in the sprite.
   */
  void set_action(const std::string& action);
  /**
   * Returns the name of the action of the image, as defined in the sprite.
   * This is only useful when the image is a sprite.
   */
  std::string get_action() const;
  /**
   * Fades in the image for the next ""time"" seconds.
   * @param float $time
   */
  void fade_in(float time);
  /**
   * Fades out the image for the next ""time"" seconds.
   * @param float $time
   */
  void fade_out(float time);

#ifndef SCRIPTING_API
private:
  FloatingImage(const FloatingImage&) = delete;
  FloatingImage& operator=(const FloatingImage&) = delete;
#endif
};

} // namespace scripting

#endif

/* EOF */
