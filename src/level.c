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
#include "globals.h"
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

  /* (Level background image) */
  fgets(str, sizeof(plevel->bkgd_image), fi);
  strcpy(plevel->bkgd_image, str);
  plevel->bkgd_image[strlen(plevel->bkgd_image)-1] = '\0';

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
      fclose(fi);
      exit(1);
    }


  /* Load the level lines: */

  for (y = 0; y < 15; y++)
    {
      if(fgets(line, plevel->width + 5, fi) == NULL)
        {
          fprintf(stderr, "Level %s isn't complete!\n",plevel->name);
          free(line);
          fclose(fi);
          exit(1);
        }
      line[strlen(line) - 1] = '\0';
      plevel->tiles[y] = strdup(line);
    }

  free(line);
  fclose(fi);

}

/* Save data for level: */

void savelevel(st_level* plevel, char * subset, int level)
{
  FILE * fi;
  char * filename;
  int y;
  char str[80];

  /* Save data file: */

  filename = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) + 20) + strlen(subset));
  sprintf(filename, "%s/levels/%s/level%d.dat", DATA_PREFIX, subset, level);
  fi = fopen(filename, "w");
  if (fi == NULL)
    {
      perror(filename);
      st_shutdown();
      free(filename);
      exit(-1);
    }
  free(filename);

  fputs(plevel->name, fi);
  fputs("\n", fi);
  fputs(plevel->theme, fi);
  fputs("\n", fi);
  sprintf(str, "%d\n", plevel->time_left);	/* time */
  fputs(str, fi);
  fputs(plevel->song_title, fi);	/* song filename */
  fputs("\n",fi);
  fputs(plevel->bkgd_image, fi);	/* background image */  
  sprintf(str, "\n%d\n", plevel->bkgd_red);	/* red background color */
  fputs(str, fi);
  sprintf(str, "%d\n", plevel->bkgd_green);	/* green background color */
  fputs(str, fi);
  sprintf(str, "%d\n", plevel->bkgd_blue);	/* blue background color */
  fputs(str, fi);
  sprintf(str, "%d\n", plevel->width);	/* level width */
  fputs(str, fi);

  for(y = 0; y < 15; ++y)
    {
      fputs(plevel->tiles[y], fi);
      fputs("\n", fi);
    }

  fclose(fi);
}


/* Unload data for this level: */

void unloadlevel(st_level* plevel)
{
  free(plevel->tiles);
  plevel->name[0] = '\0';
  plevel->theme[0] = '\0';
  plevel->song_title[0] = '\0';
  plevel->bkgd_image[0] = '\0';
}

/* Load graphics: */

void loadlevelgfx(st_level *plevel)
{

  load_level_image(&img_brick[0],plevel->theme,"brick0.png", IGNORE_ALPHA);
  load_level_image(&img_brick[1],plevel->theme,"brick1.png", IGNORE_ALPHA);

  load_level_image(&img_solid[0],plevel->theme,"solid0.png", USE_ALPHA);
  load_level_image(&img_solid[1],plevel->theme,"solid1.png", USE_ALPHA);
  load_level_image(&img_solid[2],plevel->theme,"solid2.png", USE_ALPHA);
  load_level_image(&img_solid[3],plevel->theme,"solid3.png", USE_ALPHA);

  load_level_image(&img_bkgd_tile[0][0],plevel->theme,"bkgd-00.png", USE_ALPHA);
  load_level_image(&img_bkgd_tile[0][1],plevel->theme,"bkgd-01.png", USE_ALPHA);
  load_level_image(&img_bkgd_tile[0][2],plevel->theme,"bkgd-02.png", USE_ALPHA);
  load_level_image(&img_bkgd_tile[0][3],plevel->theme,"bkgd-03.png", USE_ALPHA);

  load_level_image(&img_bkgd_tile[1][0],plevel->theme,"bkgd-10.png", USE_ALPHA);
  load_level_image(&img_bkgd_tile[1][1],plevel->theme,"bkgd-11.png", USE_ALPHA);
  load_level_image(&img_bkgd_tile[1][2],plevel->theme,"bkgd-12.png", USE_ALPHA);
  load_level_image(&img_bkgd_tile[1][3],plevel->theme,"bkgd-13.png", USE_ALPHA);

  if(strcmp(plevel->bkgd_image,"") != 0)
    {
      char fname[1024];
      snprintf(fname, 1024, "%s/background/%s", st_dir, plevel->bkgd_image);
      if(!faccessible(fname))
        snprintf(fname, 1024, "%s/images/background/%s", DATA_PREFIX, plevel->bkgd_image);
      texture_load(&img_bkgd, fname, IGNORE_ALPHA);
      printf("%s",fname);
    }
}

/* Free graphics data for this level: */

void unloadlevelgfx(void)
{
  int i;

  for (i = 0; i < 2; i++)
    {
      texture_free(&img_brick[i]);
    }
  for (i = 0; i < 4; i++)
    {
      texture_free(&img_solid[i]);
      texture_free(&img_bkgd_tile[0][i]);
      texture_free(&img_bkgd_tile[1][i]);
    }
  texture_free(&img_bkgd);
}

/* Load a level-specific graphic... */

void load_level_image(texture_type* ptexture, char* theme, char * file, int use_alpha)
{
  char fname[1024];

  snprintf(fname, 1024, "%s/themes/%s/%s", st_dir, theme, file);
  if(!faccessible(fname))
    snprintf(fname, 1024, "%s/images/themes/%s/%s", DATA_PREFIX, theme, file);

  texture_load(ptexture, fname, use_alpha);
}

/* Edit a piece of the map! */

void level_change(st_level* plevel, float x, float y, unsigned char c)
{
  int xx, yy;

  yy = ((int)y / 32);
  xx = ((int)x / 32);

  if (yy >= 0 && yy < 15 && xx >= 0 && xx <= plevel->width)
    plevel->tiles[yy][xx] = c;
}

