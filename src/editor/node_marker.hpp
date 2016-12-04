//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_NODE_MARKER_HPP
#define HEADER_SUPERTUX_NODE_MARKER_HPP

#include "editor/point_marker.hpp"
#include "object/path.hpp"

//class Path;
//class Path::Node;

class NodeMarker : public PointMarker
{
  public:
    NodeMarker(Path* path_, std::vector<Path::Node>::iterator node_iterator, size_t id_);
    ~NodeMarker();

    Path* path;
    std::vector<Path::Node>::iterator node;
    size_t id;

    void update(float elapsed_time);
    void draw(DrawingContext& context);
    void move_to(const Vector& pos);
    void editor_delete();
    Vector get_point_vector() const;
    Vector get_offset() const;

    ObjectSettings get_settings();

    void update_iterator();

  private:
    NodeMarker(const NodeMarker&);
    NodeMarker& operator=(const NodeMarker&);
};

#endif // HEADER_SUPERTUX_EDITOR_NODE_MARKER_HPP

/* EOF */
