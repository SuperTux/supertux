//
// Interface: enemy
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de> (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//


/* Types: */

typedef struct bad_guy_type
  {
    int alive;
    int mode;
    int dying;
    int timer;
    int kind;
    int seen;
    int dir;
    int x;
    int y;
    int xm;
    int ym;
  }
bad_guy_type;
