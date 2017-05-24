//  SuperTux
//  Copyright (C) 2017 Tobias Markus <tobbi.bugs@googlemail.com>
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

#ifndef HEADER_SUPERTUX_EDITOR_LAYER_ITEM_CPP
#define HEADER_SUPERTUX_EDITOR_LAYER_ITEM_CPP

/**
 * This abstract class is used for objects that
 * are displayed as layers in the editor. This is
 * a cleaner solution than defining an appropriate
 * get_layer() method on each object separately.
 **/
class LayerItem
{
private:
  /**
   * Pointer to the actual layer variable
   */
  int* layer;

public:
  /**
   * Constructor
   * @param layer Pointer to the layer we want to return
   */
  LayerItem(int* layer):
    layer(layer)
  {
  }

  /**
   * Returns the value of the actual layer
   * @return layer Number of the layer in question
   */
  int get_layer() const {
    return *layer;
  }

  /**
   * Sets the layer of this item
   * @param layer Number of the layer in question
   */
  void set_layer(int layer) {
    *(this->layer) = layer;
  }

private:
  LayerItem(const LayerItem&) = delete;
  LayerItem& operator=(const LayerItem&) = delete;
};

#endif