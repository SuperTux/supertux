//
// C++ Interface: tux
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

typedef struct upgrade_type
  {
    int alive;
    int kind;
    int height;
    int x;
    int y;
    int xm;
    int ym;
  }
upgrade_type;

typedef struct bullet_type
  {
    int alive;
    int x;
    int y;
    int xm;
    int ym;
  }
bullet_type;

