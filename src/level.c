//
// C Implementation: level
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "setup.h"
#include "screen.h"
#include "level.h"

/* Load data for this level: */

void loadlevel(st_level* plevel, char *subset, int level)
{
  int y;
  FILE * fi;
  char str[80];
  char * filename;
  char * line;

  /* Load data file: */

  filename = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) + 20) + strlen(subset));
  sprintf(filename, "%s/levels/%s/level%d.dat", DATA_PREFIX, subset, level);
  fi = fopen(filename, "r");
  if (fi == NULL)
    {
      perror(filename);
      st_shutdown();
      free(filename);
      exit(-1);
    }
  free(filename);


  /* Load header info: */


  /* (Level title) */
  fgets(str, 20, fi);
  strcpy(plevel->name, str);
  plevel->name[strlen(plevel->name)-1] = '\0';

  /* (Level theme) */
  fgets(str, 20, fi);
  strcpy(plevel->theme, str);
  plevel->theme[strlen(plevel->theme)-1] = '\0';



  /* (Time to beat level) */
  fgets(str, 10, fi);
  plevel->time_left = atoi(str);

  /* (Song file for this level) */
  fgets(str, sizeof(plevel->song_title), fi);
  strcpy(plevel->song_title, str);
  plevel->song_title[strlen(plevel->song_title)-1] = '\0';



  /* (Level background color) */
  fgets(str, 10, fi);
  plevel->bkgd_red = atoi(str);
  fgets(str, 10, fi);
  plevel->bkgd_green= atoi(str);
  fgets(str, 10, fi);
  plevel->bkgd_blue = atoi(str);

  /* (Level width) */
  fgets(str, 10, fi);
  plevel->width = atoi(str);


  /* Allocate some space for the line-reading! */

  line = (char *) malloc(sizeof(char) * (plevel->width + 5));
  if (line == NULL)
    {
      fprintf(stderr, "Couldn't allocate space to load level data!");
      exit(1);
    }


  /* Load the level lines: */

  for (y = 0; y < 15; y++)
    {
      if(fgets(line, plevel->width + 5, fi) == NULL)
        {
          fprintf(stderr, "Level %s isn't complete!\n",plevel->name);
          exit(1);
        }
      line[strlen(line) - 1] = '\0';
      plevel->tiles[y] = strdup(line);
    }

  fclose(fi);
  
}

/* Load a level-specific graphic... */

SDL_Surface * load_level_image(char* theme, char * file, int use_alpha)
{
  char fname[2024];

  snprintf(fname, 21024, "%simages/themes/%s/%s", DATA_PREFIX, theme, file);
  
  return(load_image(fname, use_alpha));
}
