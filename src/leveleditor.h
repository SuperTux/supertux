/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*  December 28, 2003 - February 1st, 2004 */

/* leveleditor.h - A built-in level editor for SuperTux
   Ricardo Cruz <rick2@aeiou.pt>                      
   Tobias Glaesser <tobi.web@gmx.de> */

#ifndef SUPERTUX_LEVELEDITOR_H
#define SUPERTUX_LEVELEDITOR_H

typedef struct square
  {
    int x1, y1, x2, y2;
  }
square;

/* selection modes */
enum {
  CURSOR,
  SQUARE
};

int leveleditor(int levelnb);
void newlevel(void);
void selectlevel(void);
void le_savelevel();
void editlevel(void);
void testlevel(void);
int le_init(void);
void le_checkevents(void);

#endif /*SUPERTUX_LEVELEDITOR_H*/
