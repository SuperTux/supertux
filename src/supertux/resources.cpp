//  SuperTux
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "supertux/resources.hpp"

#include "gui/mousecursor.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/player_status.hpp"
#include "supertux/tile_manager.hpp"
#include "video/font.hpp"

MouseCursor* Resources::mouse_cursor = NULL;

FontPtr Resources::fixed_font;
FontPtr Resources::normal_font;
FontPtr Resources::small_font;
FontPtr Resources::big_font;

SurfacePtr Resources::checkbox;
SurfacePtr Resources::checkbox_checked;
SurfacePtr Resources::back;
SurfacePtr Resources::arrow_left;
SurfacePtr Resources::arrow_right;

/* Load graphics/sounds shared between all levels: */
void
Resources::load_shared()
{
  /* Load the mouse-cursor */
  mouse_cursor = new MouseCursor("images/engine/menu/mousecursor.png",
                                 "images/engine/menu/mousecursor-click.png",
                                 "images/engine/menu/mousecursor-link.png");
  MouseCursor::set_current(mouse_cursor);

  /* Load global images: */
  fixed_font.reset(new Font(Font::FIXED, "fonts/white.stf"));
  normal_font.reset(new Font(Font::VARIABLE, "fonts/white.stf"));
  small_font.reset(new Font(Font::VARIABLE, "fonts/white-small.stf", 1));
  big_font.reset(new Font(Font::VARIABLE, "fonts/white-big.stf", 3));

  /* Load menu images */
  checkbox = Surface::create("images/engine/menu/checkbox-unchecked.png");
  checkbox_checked = Surface::create("images/engine/menu/checkbox-checked.png");
  back = Surface::create("images/engine/menu/arrow-back.png");
  arrow_left = Surface::create("images/engine/menu/arrow-left.png");
  arrow_right = Surface::create("images/engine/menu/arrow-right.png");

  tile_manager   = new TileManager();
  sprite_manager = new SpriteManager();
}

/* Free shared data: */
void
Resources::unload_shared()
{
  /* Free menu images */
  if(checkbox != NULL)
    checkbox.reset();
  if(checkbox_checked != NULL)
    checkbox_checked.reset();
  if(back != NULL)
    back.reset();
  if(arrow_left != NULL)
    arrow_left.reset();
  if(arrow_right != NULL)
    arrow_right.reset();

  /* Free global images: */
  if(fixed_font != NULL)
    fixed_font.reset();
  if(normal_font != NULL)
    normal_font.reset();
  if(small_font != NULL)
    small_font.reset();
  if(big_font != NULL)
    big_font.reset();
  
  /* Free tilesets */
  delete tile_manager;
  tile_manager = 0;

  if(sprite_manager != NULL)
  {
    delete sprite_manager;
    sprite_manager = NULL;
  }

  /* Free mouse-cursor */
  if(mouse_cursor != NULL)
  {
    delete mouse_cursor;
  }
}

/* EOF */
