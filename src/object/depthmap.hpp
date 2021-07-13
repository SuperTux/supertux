//  SuperTux - Decal
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_DEPTHMAP_HPP
#define HEADER_SUPERTUX_OBJECT_DEPTHMAP_HPP

#include "math/triangle.hpp"
#include "math/triangle3d.hpp"
#include "object/decal.hpp"
#include "video/surface_ptr.hpp"

class ReaderMapping;

/** A decorative image, perhaps part of the terrain */
class DepthMap final : public Decal
{
public:
  DepthMap(const ReaderMapping& reader);
  virtual ~DepthMap();

  virtual std::string get_class() const override { return "depthmap"; }
  virtual std::string get_display_name() const override { return _("Depthmap"); }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual void draw(DrawingContext& context) override;


private:
  std::vector<Triangle> parse_src();
  std::vector<Triangle3D> parse_dst(DrawingContext& context);

private:
  std::string m_surface_file;
  std::string m_src_data;
  std::string m_dst_data;
  SurfacePtr m_surface;

private:
  DepthMap(const DepthMap&) = delete;
  DepthMap& operator=(const DepthMap&) = delete;
};

#endif

/* EOF */
