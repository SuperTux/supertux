//
// C Interface: type
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_TYPE_H
#define SUPERTUX_TYPE_H

#include <SDL/SDL.h>

/* 'Base' type for game objects */

typedef struct base_type
  {
    int alive;
    float x;
    float y;
    float xm;
    float ym;
    float width;
    float height;
  }
base_type;

#endif /*SUPERTUX_TYPE_H*/

