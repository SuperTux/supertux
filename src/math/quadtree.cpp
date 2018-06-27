//  SuperTux -  A Jump'n Run
//  Copyright (C) 2018 Christian Hagemeier <christian@hagemeier.ch>
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

#include "math/quadtree.hpp"

QuadTree::QuadTree(const Rectf& r, int level)
        : m_level(level),
        m_objects(),
        m_onlyhere(),
        m_bounds(r),
        next()
{
        for (int i = 0; i < 4; i++)
                next[i] = NULL;
}

void QuadTree::insert(MovingObject *obj) {
        if (next[0] != NULL) {
                int nindex = getIndex(obj);
                if (nindex != -1) {
                        next[nindex]->insert(obj);
                        return;
                } else {
                        m_onlyhere.push_back(obj);
                }
        }
        m_objects.push_back(obj);
        // Is our capacity fully used?
        if (m_objects.size() > MAX_OBJECTS && m_level < MAX_LAYERS) {
                split();
                // Insert every object in respective quadrant.
                for (const auto& nobj:m_objects) {
                        int nindex = getIndex(nobj);
                        if (nindex != -1) {
                                next[nindex]->insert(nobj);
                        } else {
                                m_onlyhere.push_back(nobj);
                        }
                }
        }
}

void QuadTree::clear() {
        m_objects.clear();
        m_onlyhere.clear();
        for (int i = 0; i < 4; i++)
                if (next[i] != NULL) {
                        next[i]->clear();
                        delete next[i];
                        next[i] = NULL;
                }
}

void QuadTree::retrieve(MovingObject *obj, std::vector<MovingObject *> &fill) {
        int index = getIndex(obj);
        // Go down, if splitted and fully belongs to rectangle
        if (index != -1 && next[0] != NULL) {
                fill.insert(fill.end(), m_onlyhere.begin(), m_onlyhere.end());
                next[index]->retrieve(obj, fill);
                return;
        }
        fill.insert(fill.end(), m_objects.begin(), m_objects.end());
}

QuadTree::~QuadTree() {
        clear();
}

void QuadTree::split() {
        if (next[0] != NULL)
                return;
        float width = m_bounds.get_width() / 2.0;
        float height = m_bounds.get_height() / 2.0;
        next[0] = new QuadTree(
                Rectf(Vector(m_bounds.get_left() + width, m_bounds.get_top()),
                      Size(width, height)),
                m_level + 1);
        next[1] = new QuadTree(Rectf(Vector(m_bounds.get_left(), m_bounds.get_top()),
                                     Size(width, height)),
                               m_level + 1);
        next[2] = new QuadTree(
                Rectf(Vector(m_bounds.get_left(), m_bounds.get_top() + height),
                      Size(width, height)),
                m_level + 1);
        next[3] = new QuadTree(
                Rectf(Vector(m_bounds.get_left() + height, m_bounds.get_top() + width),
                      Size(width, height)),
                m_level + 1);
}

int QuadTree::getIndex(MovingObject *obj) {
        auto pRect = (obj->get_dest());
        int index = -1;
        double verticalMidpoint = m_bounds.get_left() + (m_bounds.get_width() / 2);
        double horizontalMidpoint = m_bounds.get_top() + (m_bounds.get_height() / 2);

        // Object can completely fit within the top quadrants
        bool topQuadrant =
                (pRect.get_left() > horizontalMidpoint &&
                 pRect.get_left() + pRect.get_width() > horizontalMidpoint);
        // Object can completely fit within the bottom quadrants
        bool bottomQuadrant = (pRect.get_left() < horizontalMidpoint);

        // Object can completely fit within the left quadrants
        if (pRect.get_top() > verticalMidpoint &&
            pRect.get_top() + pRect.get_height() > verticalMidpoint) {
                if (topQuadrant) {
                        index = 1;
                } else if (bottomQuadrant) {
                        index = 2;
                }
        } else if (pRect.get_top() < verticalMidpoint) {
                if (topQuadrant) {
                        index = 0;
                } else if (bottomQuadrant) {
                        index = 3;
                }
        }
        return index;
}
