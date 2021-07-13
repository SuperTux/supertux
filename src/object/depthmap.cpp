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

#include "object/depthmap.hpp"

#include "sprite/sprite_manager.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"
#include "video/surface.hpp"

#include <regex>

DepthMap::DepthMap(const ReaderMapping& reader) :
  Decal(reader),
  m_surface_file(),
  m_src_data(),
  m_dst_data(),
  m_surface()
{
  reader.get("surface", m_surface_file);
  reader.get("srcdata", m_src_data);
  reader.get("dstdata", m_dst_data);

  m_surface = Surface::from_file(m_surface_file);
}

ObjectSettings
DepthMap::get_settings()
{
  ObjectSettings result = Decal::get_settings();

  result.remove("sprite");

  result.add_surface(_("Surface"), &m_surface_file, "surface");
  result.add_text(_("Image triangle data"), &m_src_data, "srcdata", std::string("0;0;0;1;1;0;0;1;1;0;1;1"));
  result.add_text(_("Level triangle data"), &m_dst_data, "dstdata", std::string("0;0;1;0;1;1;1;0;1;0;1;1;1;0;1;1;1;1"));

  return result;
}

void
DepthMap::after_editor_set()
{
  Decal::after_editor_set();
  m_surface = Surface::from_file(m_surface_file);
}

DepthMap::~DepthMap()
{
}

void
DepthMap::draw(DrawingContext& context)
{
  if (!m_visible)
    return;

  try
  {
    auto src = parse_src();
    auto dst = parse_dst(context);

    if (src.size() != dst.size())
      throw std::runtime_error("DepthMap Object draw(): src and dst of different sizes");

    context.color().draw_depthmap(m_surface, src, dst, Color::WHITE, m_layer);
  }
  catch(std::exception& e)
  {
    log_warning << e.what() << std::endl;
  }
}

std::vector<Triangle>
DepthMap::parse_src()
{
  std::stringstream str(m_src_data);
  std::string segment;
  std::vector<float> nlist;
  std::vector<Triangle> tris;

  while(std::getline(str, segment, ';'))
    nlist.push_back(std::stof(segment));

  if (nlist.size() % 6 != 0)
    throw std::runtime_error("Incorrectly formatted source list");
  
  auto it = nlist.begin();
  while (it != nlist.end())
  {
    float x1 = *(it++), y1 = *(it++),
          x2 = *(it++), y2 = *(it++),
          x3 = *(it++), y3 = *(it++);
    tris.push_back(Triangle(Vector(x1, y1),Vector(x2, y2),Vector(x3, y3)));
  }

  return tris;
}

std::vector<Triangle3D>
DepthMap::parse_dst(DrawingContext& context)
{
  std::stringstream str(m_dst_data);
  std::string segment;
  std::vector<float> nlist;
  std::vector<Triangle3D> tris;

  while(std::getline(str, segment, ';'))
    nlist.push_back(std::stof(segment));

  if (nlist.size() % 9 != 0)
    throw std::runtime_error("Incorrectly formatted destination list");

  auto it = nlist.begin();
  while (it != nlist.end())
  {
    float x1 = *(it++), y1 = *(it++), z1 = *(it++),
          x2 = *(it++), y2 = *(it++), z2 = *(it++),
          x3 = *(it++), y3 = *(it++), z3 = *(it++);
    tris.push_back(Triangle3D(Vector3(x1, y1, z1),
                              Vector3(x2, y2, z2),
                              Vector3(x3, y3, z3))
                    .moved(m_col.m_bbox.p1()));
  }

  return tris;
}

/* EOF */
