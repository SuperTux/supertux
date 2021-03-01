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

#ifndef HEADER_SUPERTUX_EDITOR_LAYER_ICON_HPP
#define HEADER_SUPERTUX_EDITOR_LAYER_ICON_HPP

#include "editor/object_icon.hpp"

class GameObject;
class Vector;

class LayerIcon : public ObjectIcon
{
public:
  LayerIcon(GameObject* layer);
  virtual ~LayerIcon() {}

  virtual void draw(DrawingContext& context, const Vector& pos) override;
  virtual void draw(DrawingContext& context, const Vector& pos, int pixels_shown) override;

  int get_zpos() const;
  bool is_valid() const;

  GameObject* get_layer() const { return m_layer; }
  bool is_tilemap() const;

private:
  GameObject* m_layer;
  SurfacePtr m_selection;

private:
  LayerIcon(const LayerIcon&) = delete;
  LayerIcon& operator=(const LayerIcon&) = delete;
};

#endif

/* EOF */
