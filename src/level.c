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
#include "physic.h"
#include "scene.h"

texture_type img_bkgd, img_bkgd_tile[2][4], img_solid[4], img_brick[2];

void subset_init(st_subset* st_subset)
{
  st_subset->title = NULL;
  st_subset->description = NULL;
  st_subset->name = NULL;
  st_subset->levels = 0;
}

void subset_load(st_subset* st_subset, char *subset)
{
  FILE* fi;
  char filename[1024];
  char str[1024];
  int len,i;

  st_subset->name = (char*) malloc(sizeof(char)*(strlen(subset)+1));
  strcpy(st_subset->name,subset);

  snprintf(filename, 1024, "%s/levels/%s/info", st_dir, subset);
  if(!faccessible(filename))
    snprintf(filename, 1024, "%s/levels/%s/info", DATA_PREFIX, subset);
  if(faccessible(filename))
    {
      fi = fopen(filename, "r");
      if (fi == NULL)
        {
          perror(filename);
        }

      /* Load title info: */
      fgets(str, 40, fi);
      st_subset->title = (char*) malloc(sizeof(char)*(strlen(str)+1));
      strcpy(st_subset->title, str);

      /* Load the description: */

      str[0] = '\0';
      st_subset->description = NULL;
      len = 0;
      while(fgets(str, 300, fi) != NULL)
        {
          len += strlen(str);
          if(st_subset->description == NULL)
            st_subset->description = (char*) calloc(len+1,sizeof(char));
          else
            st_subset->description = (char*) realloc(st_subset->description, sizeof(char) * (len+1));
          strcat(st_subset->description,str);
        }
      fclose(fi);

      snprintf(str, 1024, "%s.png", filename);
      if(faccessible(str))
        {
          texture_load(&st_subset->image,str,IGNORE_ALPHA);
        }
      else
        {
          snprintf(filename, 1024, "%s/images/status/level-subset-info.png", DATA_PREFIX);
          texture_load(&st_subset->image,filename,IGNORE_ALPHA);
        }
    }

  for(i=1; i != -1; ++i)
    {
      /* Get the number of levels in this subset */
      snprintf(filename, 1024, "%s/levels/%s/level%d.dat", st_dir, subset,i);
      if(!faccessible(filename))
        {
          snprintf(filename, 1024, "%s/levels/%s/level%d.dat", DATA_PREFIX, subset,i);
          if(!faccessible(filename))
            break;
        }
    }
  st_subset->levels = --i;
}

void subset_save(st_subset* st_subset)
{
  FILE* fi;
  char filename[1024];

  /* Save data file: */
  sprintf(filename, "/levels/%s/", st_subset->name);

  fcreatedir(filename);
  snprintf(filename, 1024, "%s/levels/%s/info", st_dir, st_subset->name);
  if(!fwriteable(filename))
    snprintf(filename, 1024, "%s/levels/%s/info", DATA_PREFIX, st_subset->name);
  if(fwriteable(filename))
    {
      fi = fopen(filename, "w");
      if (fi == NULL)
        {
          perror(filename);
        }

      /* Save title info: */
      fputs(st_subset->title, fi);
      fputs("\n", fi);

      /* Save the description: */

      fputs(st_subset->description, fi);
      fputs("\n", fi);
      fclose(fi);

    }
}

void subset_free(st_subset* st_subset)
{
  free(st_subset->title);
  free(st_subset->description);
  free(st_subset->name);
  texture_free(&st_subset->image);
  st_subset->levels = 0;
}

/* Load data for this level: */
/* Returns -1, if the loading of the level failed. */
int level_load(st_level* plevel, char *subset, int level)
{
  int y,x;
  FILE * fi;
  char str[80];
  char filename[1024];
  char * line;

  /* Load data file: */

  snprintf(filename, 1024, "%s/levels/%s/level%d.dat", st_dir, subset, level);
  if(!faccessible(filename))
    snprintf(filename, 1024, "%s/levels/%s/level%d.dat", DATA_PREFIX, subset, level);
  fi = fopen(filename, "r");
  if (fi == NULL)
    {
      perror(filename);
      return -1;
    }


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

  /* (Level gravity) */
  fgets(str, 10, fi);
  plevel->gravity = atof(str);

  /* Set the global gravity to the latest loaded level's gravity */
  gravity = plevel->gravity;

  /* Allocate some space for the line-reading! */

  line = (char *) malloc(sizeof(char) * (plevel->width + 5));
  if (line == NULL)
    {
      fprintf(stderr, "Couldn't allocate space to load level data!");
      fclose(fi);
      return -1;
    }


  /* Load the level lines: */

  for (y = 0; y < 15; y++)
    {
      if(fgets(line, plevel->width + 5, fi) == NULL)
        {
          fprintf(stderr, "Level %s isn't complete!\n",plevel->name);
          free(line);
          fclose(fi);
          return -1;
        }
      line[strlen(line) - 1] = '\0';
      plevel->tiles[y] = (unsigned char*) strdup(line);
    }

  /*  Mark the end position of this level! - Is a bit wrong here thought */

  for (y = 0; y < 15; ++y)
    {
      for (x = 0; x < plevel->width; ++x)
        {
          if(plevel->tiles[y][x] == '|')
	  {
	  if(x*32 > endpos)
	  endpos = x*32;
	  }
        }
    }

  free(line);
  fclose(fi);
  return 0;
}

/* Save data for level: */

void level_save(st_level* plevel, char * subset, int level)
{
  FILE * fi;
  char filename[1024];
  int y;
  char str[80];

  /* Save data file: */
  sprintf(str, "/levels/%s/", subset);
  fcreatedir(str);
  snprintf(filename, 1024, "%s/levels/%s/level%d.dat", st_dir, subset, level);
  if(!fwriteable(filename))
    snprintf(filename, 1024, "%s/levels/%s/level%d.dat", DATA_PREFIX, subset, level);

  fi = fopen(filename, "w");
  if (fi == NULL)
    {
      perror(filename);
      st_shutdown();
      exit(-1);
    }

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
  sprintf(str, "%2.1f\n", plevel->gravity);	/* level gravity */
  fputs(str, fi);

  for(y = 0; y < 15; ++y)
    {
      fputs((const char*)plevel->tiles[y], fi);
      fputs("\n", fi);
    }

  fclose(fi);
}


/* Unload data for this level: */

void level_free(st_level* plevel)
{
  int i;
  for(i=0; i < 15; ++i)
    free(plevel->tiles[i]);

  plevel->name[0] = '\0';
  plevel->theme[0] = '\0';
  plevel->song_title[0] = '\0';
  plevel->bkgd_image[0] = '\0';
}

/* Load graphics: */

void level_load_gfx(st_level *plevel)
{
  level_load_image(&img_brick[0],plevel->theme,"brick0.png", IGNORE_ALPHA);
  level_load_image(&img_brick[1],plevel->theme,"brick1.png", IGNORE_ALPHA);

  level_load_image(&img_solid[0],plevel->theme,"solid0.png", USE_ALPHA);
  level_load_image(&img_solid[1],plevel->theme,"solid1.png", USE_ALPHA);
  level_load_image(&img_solid[2],plevel->theme,"solid2.png", USE_ALPHA);
  level_load_image(&img_solid[3],plevel->theme,"solid3.png", USE_ALPHA);

  level_load_image(&img_bkgd_tile[0][0],plevel->theme,"bkgd-00.png", USE_ALPHA);
  level_load_image(&img_bkgd_tile[0][1],plevel->theme,"bkgd-01.png", USE_ALPHA);
  level_load_image(&img_bkgd_tile[0][2],plevel->theme,"bkgd-02.png", USE_ALPHA);
  level_load_image(&img_bkgd_tile[0][3],plevel->theme,"bkgd-03.png", USE_ALPHA);

  level_load_image(&img_bkgd_tile[1][0],plevel->theme,"bkgd-10.png", USE_ALPHA);
  level_load_image(&img_bkgd_tile[1][1],plevel->theme,"bkgd-11.png", USE_ALPHA);
  level_load_image(&img_bkgd_tile[1][2],plevel->theme,"bkgd-12.png", USE_ALPHA);
  level_load_image(&img_bkgd_tile[1][3],plevel->theme,"bkgd-13.png", USE_ALPHA);

  if(strcmp(plevel->bkgd_image,"") != 0)
    {
      char fname[1024];
      snprintf(fname, 1024, "%s/background/%s", st_dir, plevel->bkgd_image);
      if(!faccessible(fname))
        snprintf(fname, 1024, "%s/images/background/%s", DATA_PREFIX, plevel->bkgd_image);
      texture_load(&img_bkgd, fname, IGNORE_ALPHA);
    }
  else
    {
      /* Quick hack to make sure an image is loaded, when we are freeing it afterwards. */#
      level_load_image(&img_bkgd, plevel->theme,"solid0.png", IGNORE_ALPHA);
    }
}

/* Free graphics data for this level: */

void level_free_gfx(void)
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

void level_load_image(texture_type* ptexture, char* theme, char * file, int use_alpha)
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

/* Free music data for this level: */

void level_free_song(void)
{
  free_music(level_song);
  free_music(level_song_fast);
}

/* Load music: */

void level_load_song(st_level* plevel)
{

  char * song_path;
  char * song_subtitle;

  song_path = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) +
                              strlen(plevel->song_title) + 8));
  sprintf(song_path, "%s/music/%s", DATA_PREFIX, plevel->song_title);
  level_song = load_song(song_path);
  free(song_path);


  song_path = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) +
                              strlen(plevel->song_title) + 8 + 5));
  song_subtitle = strdup(plevel->song_title);
  strcpy(strstr(song_subtitle, "."), "\0");
  sprintf(song_path, "%s/music/%s-fast%s", DATA_PREFIX, song_subtitle, strstr(plevel->song_title, "."));
  level_song_fast = load_song(song_path);
  free(song_subtitle);
  free(song_path);
}
