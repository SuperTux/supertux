//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "video/painter.hpp"

#include <optional>

class NullPainter : public Painter
{
public:
  NullPainter();
  ~NullPainter() override;

  virtual void draw_texture(const DrawingRequest& request) override;
  virtual void draw_gradient(const DrawingRequest& request) override;
  virtual void draw_filled_rect(const DrawingRequest& request) override;
  virtual void draw_inverse_ellipse(const DrawingRequest& request) override;
  virtual void draw_line(const DrawingRequest& request) override;
  virtual void draw_triangle(const DrawingRequest& request) override;

  virtual void clear(const Color& color) override;
  virtual void get_pixel(const DrawingRequest& request) const override;

  virtual void set_clip_rect(const Rect& rect) override;
  virtual void clear_clip_rect() override;

private:
  std::optional<Rect> m_clip_rect;

private:
  NullPainter(const NullPainter&) = delete;
  NullPainter& operator=(const NullPainter&) = delete;
};
