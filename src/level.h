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

typedef struct st_level /*It is easier to read the sources IMHO, if we don't write something like int a,b,c; */
  {
    char name[100];
    char theme[100];
    char song_title[100];
    unsigned char* tiles[15];
    int time_left;
    int bkgd_red;
    int bkgd_green;
    int bkgd_blue;
    int width;
  } st_level;

void loadlevel(st_level* plevel, char * subset, int level);
SDL_Surface * load_level_image(char* theme, char * file, int use_alpha);


