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

#ifndef HEADER_SUPERTUX_EDITOR_LAYERS_GUI_HPP
#define HEADER_SUPERTUX_EDITOR_LAYERS_GUI_HPP

#include <stdexcept>

#include "control/input_manager.hpp"
#include "supertux/screen.hpp"

class EditorLayersGui
{
  public:
    EditorLayersGui();
    ~EditorLayersGui();

    void draw(DrawingContext&);
    void update(float elapsed_time);
    bool event(SDL_Event& ev);
    void setup();

    void refresh_sector_text();

  private:
    int Ypos;
    const int Xpos = 32;
    int Width;

    std::string sector_text;
    int sector_text_width;

    typedef enum {
      HI_NONE, HI_SPAWNPOINTS, HI_SECTOR, HI_LAYERS
    }HoveredItem;

    HoveredItem hovered_item;

};

#endif // HEADER_SUPERTUX_EDITOR_LAYERS_GUI_HPP
