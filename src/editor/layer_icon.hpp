//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#ifndef LAYER_ICON_HPP
#define LAYER_ICON_HPP

#include "editor/object_icon.hpp"

class GameObject;

class LayerIcon : public ObjectIcon
{
  public:
    LayerIcon(GameObject *layer_);
    virtual ~LayerIcon();

    GameObject *layer;

    virtual void draw(DrawingContext& context, Vector pos);

    int get_zpos();
    bool is_valid() const;

    bool is_tilemap;

  private:

    SurfacePtr selection;

    LayerIcon(const LayerIcon&);
    LayerIcon& operator=(const LayerIcon&);
};

#endif // LAYER_ICON_HPP

/* EOF */
