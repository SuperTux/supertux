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

#ifndef HEADER_SUPERTUX_OBJECT_BACKGROUND_HPP
#define HEADER_SUPERTUX_OBJECT_BACKGROUND_HPP

#include "math/vector.hpp"
#include "scripting/background.hpp"
#include "squirrel/exposed_object.hpp"
#include "supertux/game_object.hpp"
#include "supertux/timer.hpp"
#include "video/blend.hpp"
#include "video/drawing_context.hpp"
#include "video/surface_ptr.hpp"

class ReaderMapping;

class Background final : public GameObject,
                         public ExposedObject<Background, scripting::Background>
{
public:
  Background();
  Background(const ReaderMapping& reader);
  ~Background() override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  virtual std::string get_class() const override { return "background"; }
  virtual std::string get_display_name() const override { return _("Background"); }

  virtual const std::string get_icon_path() const override {
    return "images/engine/editor/background.png";
  }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  void set_image(const std::string& name);
  void set_images(const std::string& name_top, const std::string& name_middle, const std::string& name_bottom);
  void set_speed(float bgd_speed);

  void draw_image(DrawingContext& context, const Vector& pos);

  std::string get_image() const { return m_imagefile; }
  float get_speed() const { return m_parallax_speed.x; }
  int get_layer() const { return m_layer; }

  Color get_color() const { return m_color; }
  void set_color(Color color) { m_color = color; }
  void fade_color(Color color, float time);

private:
  enum Alignment {
    NO_ALIGNMENT,
    LEFT_ALIGNMENT,
    RIGHT_ALIGNMENT,
    TOP_ALIGNMENT,
    BOTTOM_ALIGNMENT
  };

private:
  SurfacePtr load_background(const std::string& image_path);

private:
  /** Backgrounds with NO_ALIGNMENT are repeated over the whole
      screen, backgrounds with left, right, top, bottom alignment are
      only repeated in one direction and attached to the level edge. */
  Alignment m_alignment;

  /** If fill is set, the background will not repeat and is instead
      stretched over the whole screen, alignment and top/bottom images
      are ignored in that case. */
  bool m_fill;

  int m_layer;
  std::string m_imagefile_top;
  std::string m_imagefile;
  std::string m_imagefile_bottom;
  Vector m_pos; /**< coordinates of upper-left corner of image */
  Vector m_parallax_speed;
  Vector m_scroll_speed;
  Vector m_scroll_offset;
  SurfacePtr m_image_top; /**< image to draw above pos */
  SurfacePtr m_image; /**< image to draw, anchored at pos */
  SurfacePtr m_image_bottom; /**< image to draw below pos+screenheight */

  bool m_has_pos_x;
  bool m_has_pos_y;

  Blend m_blend;
  Color m_color;
  DrawingTarget m_target;

  Timer m_timer_color;
  Color m_src_color, m_dst_color;

private:
  Background(const Background&) = delete;
  Background& operator=(const Background&) = delete;
};

#endif

/* EOF */
