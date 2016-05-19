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

#ifndef HEADER_SUPERTUX_GUI_MENU_ACTION_HPP
#define HEADER_SUPERTUX_GUI_MENU_ACTION_HPP

/* Kinds of menu items */
enum MenuItemKind {
  MN_ACTION,
  MN_GOTO,
  MN_TOGGLE,
  MN_BACK,
  MN_INACTIVE,
  MN_TEXTFIELD,
  MN_NUMFIELD,
  MN_INTFIELD,
  MN_CONTROLFIELD,
  MN_STRINGSELECT,
  MN_BADGUYSELECT,
  MN_LABEL,
  MN_COLOR,
  MN_SCRIPT,
  MN_FILE,
  MN_HL /* horizontal line */
};

/* Action done on the menu */
enum MenuAction {
  MENU_ACTION_NONE = -1,
  MENU_ACTION_UP,
  MENU_ACTION_DOWN,
  MENU_ACTION_LEFT,
  MENU_ACTION_RIGHT,
  MENU_ACTION_HIT,
  MENU_ACTION_INPUT,
  MENU_ACTION_REMOVE,
  MENU_ACTION_BACK
};

#endif // HEADER_SUPERTUX_GUI_MENU_ACTION_HPP

/* EOF */
