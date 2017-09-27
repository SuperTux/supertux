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

#ifndef HEADER_SUPERTUX_OBJECT_GRADIENT_HPP
#define HEADER_SUPERTUX_OBJECT_GRADIENT_HPP

#include "scripting/exposed_object.hpp"
#include "scripting/gradient.hpp"
#include "supertux/game_object.hpp"
#include "video/drawing_context.hpp"

class ReaderMapping;

class Gradient : public GameObject,
                 public ExposedObject<Gradient, scripting::Gradient>
{
public:
  Gradient();
  Gradient(const ReaderMapping& reader);
  virtual ~Gradient();
  virtual bool do_save() const;
  virtual void save(Writer& writer);

  void set_gradient(Color top, Color bottom);

  Color get_gradient_top() const
  { return gradient_top; }

  Color get_gradient_bottom() const
  { return gradient_bottom; }

  GradientDirection get_direction() const
  { return gradient_direction; }

  void set_direction(const GradientDirection& direction);

  virtual void update(float elapsed_time);

  virtual void draw(DrawingContext& context);

  void on_window_resize();

  std::string get_class() const {
    return "gradient";
  }

  std::string get_display_name() const {
    return _("Gradient");
  }

  int get_layer() const
  { return layer; }

  virtual ObjectSettings get_settings();

  virtual const std::string get_icon_path() const {
    return "images/engine/editor/gradient.png";
  }

private:
  int layer;
  Color gradient_top;
  Color gradient_bottom;
  GradientDirection gradient_direction;
  Rectf gradient_region;
};

#endif /*SUPERTUX_BACKGROUND_H*/

/* EOF */
