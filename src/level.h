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

#include <string>
#include "texture.h"
#include "lispreader.h"

/* This type holds meta-information about a level-subset. */
/* It could be extended to handle manipulation of subsets. */
class st_subset
  {
  public:
    st_subset();
    static void create(const std::string& subset_name);
    void load(char *subset);
    void save();
    void free();

    std::string name;
    std::string title;
    std::string description;
    texture_type image;
    int levels;
 
  private:
    void parse(lisp_object_t* cursor);
  };

#define LEVEL_NAME_MAX 20

typedef struct st_level 
{
  std::string name;
  std::string theme;
  std::string song_title;
  std::string bkgd_image;
  unsigned int* tiles[15];
  int time_left;
  int bkgd_red;
  int bkgd_green;
  int bkgd_blue;
  int width;
  float gravity;
}
st_level;

extern texture_type img_bkgd, img_bkgd_tile[2][4], img_solid[4], img_brick[2];

void level_default  (st_level* plevel);
int  level_load     (st_level* plevel, const char * subset, int level);
void level_parse    (st_level* plevel, lisp_object_t* cursor);
int  level_load     (st_level* plevel, const char* filename);
void level_save     (st_level* plevel, const char * subset, int level);
void level_free     (st_level* plevel);
void level_load_gfx (st_level* plevel);
void level_change   (st_level* plevel, float x, float y, unsigned char c);
void level_load_song(st_level* plevel);
void level_free_gfx();
void level_load_image(texture_type* ptexture, std::string theme, const char * file, int use_alpha);
void level_free_song(void);

#endif /*SUPERTUX_LEVEL_H*/
