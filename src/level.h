//
// C Interface: level
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_LEVEL_H
#define SUPERTUX_LEVEL_H

#include "texture.h"

#define LEVEL_NAME_MAX 20

typedef struct st_level /*It is easier to read the sources IMHO, if we don't write something like int a,b,c; */
  {
    char name[LEVEL_NAME_MAX];
    char theme[100];
    char song_title[100];
    unsigned char* tiles[15];
    int time_left;
    int bkgd_red;
    int bkgd_green;
    int bkgd_blue;
    int width;
  } st_level;

texture_type img_bkgd[2][4], img_solid[4], img_brick[2];
  
void loadlevel(st_level* plevel, char * subset, int level);
void loadlevelgfx(st_level* plevel);
void unloadlevelgfx();
void load_level_image(texture_type* ptexture, char* theme, char * file, int use_alpha);
void level_change(st_level* plevel, float x, float y, unsigned char c);

#endif /*SUPERTUX_LEVEL_H*/
