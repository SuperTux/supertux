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

#include "scripting/gradient.hpp"
#include "squirrel/exposed_object.hpp"
#include "supertux/game_object.hpp"
#include "video/drawing_context.hpp"

class ReaderMapping;

class Gradient final :
  public GameObject,
  public ExposedObject<Gradient, scripting::Gradient>
{
public:
  Gradient();
  Gradient(const ReaderMapping& reader);
  ~Gradient() override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  virtual bool is_saveable() const override;

  virtual std::string get_class() const override { return "gradient"; }
  virtual std::string get_display_name() const override { return _("Gradient"); }

  virtual const std::string get_icon_path() const override {
    return "images/engine/editor/gradient.png";
  }

  virtual ObjectSettings get_settings() override;

  void set_gradient(Color top, Color bottom);
  void fade_gradient(Color top, Color bottom, float time);
  Color get_gradient_top() const { return m_gradient_top; }
  Color get_gradient_bottom() const { return m_gradient_bottom; }

  GradientDirection get_direction() const { return m_gradient_direction; }
  void set_direction(const GradientDirection& direction);

  void set_layer(int layer) { m_layer = layer; }
  int get_layer() const { return m_layer; }

private:
  int m_layer;
  Color m_gradient_top;
  Color m_gradient_bottom;
  GradientDirection m_gradient_direction;
  Blend m_blend;
  DrawingTarget m_target;

private:
  Gradient(const Gradient&) = delete;
  Gradient& operator=(const Gradient&) = delete;
  
  Color m_start_gradient_top;
  Color m_start_gradient_bottom;
  Color m_fade_gradient_top;
  Color m_fade_gradient_bottom;
  float m_fade_total_time;
  float m_fade_time = 0;
};

#endif

/* EOF */
