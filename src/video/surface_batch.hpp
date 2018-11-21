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

#ifndef HEADER_SUPERTUX_VIDEO_SURFACE_BATCH_HPP
#define HEADER_SUPERTUX_VIDEO_SURFACE_BATCH_HPP

#include <vector>

#include "video/paint_style.hpp"
#include "video/surface_ptr.hpp"

class Rectf;
class Vector;

class SurfaceBatch
{
public:
  SurfaceBatch(const SurfacePtr& surface, const PaintStyle& style);
  SurfaceBatch(const SurfaceBatch&) = default;
  SurfaceBatch& operator=(const SurfaceBatch&) = default;

  void draw(const Vector& pos, float angle = 0.0f);
  void draw(const Rectf& dstrect, float angle = 0.0f);
  void draw(const Rectf& srcrect, const Rectf& dstrect, float angle = 0.0f);

  const std::vector<Rectf>& get_srcrects() const { return m_srcrects; }
  const std::vector<Rectf>& get_dstrects() const { return m_dstrects; }
  const std::vector<float>& get_angles() const { return m_angles; }

private:
  SurfacePtr m_surface;
  PaintStyle m_style;
  std::vector<Rectf> m_srcrects;
  std::vector<Rectf> m_dstrects;
  std::vector<float> m_angles;
};

#endif

/* EOF */
