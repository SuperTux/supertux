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

/* This type holds meta-information about a level-subset. */
/* It could be extended to handle manipulation of subsets. */
typedef struct st_subset
  {
    char *name;
    char *title;
    char *description;
    texture_type image;
    int levels;
  } st_subset;

void subset_init(st_subset* st_subset);
void subset_load(st_subset* st_subset, char *subset);
void subset_save(st_subset* st_subset);
void subset_free(st_subset* st_subset);
  
#define LEVEL_NAME_MAX 20

typedef struct st_level /*It is easier to read the sources IMHO, if we don't write something like int a,b,c; */
  {
    char name[LEVEL_NAME_MAX];
    char theme[100];
    char song_title[100];
    char bkgd_image[100];   
    unsigned char* tiles[15];
    int time_left;
    int bkgd_red;
    int bkgd_green;
    int bkgd_blue;
    int width;
    float gravity;
  } st_level;
  
extern texture_type img_bkgd, img_bkgd_tile[2][4], img_solid[4], img_brick[2];
  
int level_load(st_level* plevel, char * subset, int level);
void level_save(st_level* plevel, char * subset, int level);
void level_free(st_level* plevel);
void level_load_gfx(st_level* plevel);
void level_free_gfx();
void level_load_image(texture_type* ptexture, char* theme, char * file, int use_alpha);
void level_change(st_level* plevel, float x, float y, unsigned char c);
void level_load_song(st_level* plevel);
void level_free_song(void);

#endif /*SUPERTUX_LEVEL_H*/
