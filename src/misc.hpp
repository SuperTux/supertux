//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Tobas Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

enum MainMenuIDs {
  MNID_STARTGAME,
  MNID_LEVELS_CONTRIB,
  MNID_OPTIONMENU,
  MNID_LEVELEDITOR,
  MNID_CREDITS,
  MNID_QUITMAINMENU
};

/* Create and setup menus. */
void setup_menu();
void free_menu();

#endif
