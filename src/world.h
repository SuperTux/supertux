//
// Interface: world
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

typedef struct bouncy_distro_type /*It is easier to read the sources IMHO, if we don't write something like int a,b,c; */
  {
    int alive;
    int x;
    int y;
    int ym;
  }
bouncy_distro_type;

typedef struct broken_brick_type
  {
    int alive;
    int x;
    int y;
    int xm;
    int ym;
  }
broken_brick_type;

typedef struct bouncy_brick_type
  {
    int alive;
    int x;
    int y;
    int offset;
    int offset_m;
    int shape;
  }
bouncy_brick_type;

typedef struct floating_score_type
  {
    int alive;
    int timer;
    int x;
    int y;
    int value;
  }
floating_score_type;

