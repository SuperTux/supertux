//  SuperTux
//  Copyright (C) 2004 Tobas Glaesser <tobi.web@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SUPERTUX_MISC_H
#define SUPERTUX_MISC_H

#include "app/setup.h"
#include "app/gettext.h"
#include "gui/menu.h"
#include "utils/configfile.h"
#include "title.h"
#include "resources.h"
#include "worldmap.h"
#include "gameloop.h"
#include "object/player.h"

class MyConfig : public Config
{
  public:
    void customload(const lisp::Lisp& reader);
    void customsave(FILE * config);
};

enum OptionsMenuIDs {
  MNID_OPENGL,
  MNID_FULLSCREEN,
  MNID_SOUND,
  MNID_MUSIC,
  MNID_SHOWFPS
  };

/* Handle changes made to global settings in the options menu. */
void process_options_menu();

/* Create and setup menus. */
void st_menu();
void st_menu_free();

#endif //SUPERTUX_MISC_H

