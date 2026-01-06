//  SuperTux
//  Copyright (C) 2026 Hyland B. <me@ow.swag.toys>
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

#pragma once

#include "video/color.hpp"
#include "supertux/moving_object.hpp"

class DraggableRegion : public MovingObject
{
public:
  explicit DraggableRegion(Color color);
  explicit DraggableRegion(const ReaderMapping &reader);
  DraggableRegion(Color color, const ReaderMapping &reader);
  virtual ~DraggableRegion() {}

  void draw_draggable_box(DrawingContext& context);

  virtual bool can_be_hidden() const { return true; }
  virtual int get_layer() const override { return LAYER_OBJECTS; }
  virtual bool has_variable_size() const override { return true; }

  virtual void draw(DrawingContext& context) override
  {
    draw_draggable_box(context);
  }

  void set_color(Color color) { m_color = std::move(color); }

private:
  Color m_color;
};
