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

#ifndef HEADER_SUPERTUX_OBJECT_FLOATING_IMAGE_HPP
#define HEADER_SUPERTUX_OBJECT_FLOATING_IMAGE_HPP

#include "math/anchor_point.hpp"
#include "sprite/sprite_ptr.hpp"
#include "supertux/game_object.hpp"

class FloatingImage final : public GameObject
{
public:
  static void register_class(ssq::VM& vm);

public:
  FloatingImage(const std::string& sprite);
  ~FloatingImage() override;

  virtual bool is_saveable() const override { return false; }
  virtual std::string get_exposed_class_name() const override { return "FloatingImage"; }

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  /**
   * @deprecated Use the ""layer"" property instead!
   * Sets the layer of the floating image.
   * @param int $layer
   */
  void set_layer(int layer);
  /**
   * @deprecated Use the ""layer"" property instead!
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
  float get_x() const;
  /**
   * Returns the image's Y coordinate relative to the current anchor point.
   */
  float get_y() const;
#ifdef DOXYGEN_SCRIPTING
  /**
   * @deprecated Use ""get_x()"" instead!
   * Returns the image's X coordinate relative to the current anchor point.
   */
  float get_pos_x() const;
  /**
   * @deprecated Use ""get_y()"" instead!
   * Returns the image's Y coordinate relative to the current anchor point.
   */
  float get_pos_y() const;
#endif
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
   * @deprecated Use the ""visible"" property instead!
   * Sets the visibility of the floating image.
   * @param bool $visible
   */
  void set_visible(bool visible);
  /**
   * @deprecated Use the ""visible"" property instead!
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

private:
  SpritePtr m_sprite;
  int m_layer;
  bool m_visible;
  AnchorPoint m_anchor;
  Vector m_pos;
  float m_fading;
  float m_fadetime;
};

#endif

/* EOF */
