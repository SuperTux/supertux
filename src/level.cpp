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

#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "globals.h"
#include "setup.h"
#include "screen.h"
#include "level.h"
#include "physic.h"
#include "scene.h"
#include "lispreader.h"

using namespace std;

texture_type img_bkgd, img_bkgd_tile[2][4], img_solid[4], img_brick[2];

st_subset::st_subset()
{
  levels = 0;
}

void st_subset::create(const std::string& subset_name)
{
  st_level new_lev;
  st_subset new_subset;
  new_subset.name = subset_name;
  new_subset.title = "Unknown Title";
  new_subset.description = "No description so far.";
  new_subset.save();
  level_default(&new_lev);
  level_save(&new_lev,subset_name.c_str(),1);
}

void st_subset::parse (lisp_object_t* cursor)
{
  while(!lisp_nil_p(cursor))
    {
      lisp_object_t* cur = lisp_car(cursor);
      char *s;

      if (!lisp_cons_p(cur) || !lisp_symbol_p (lisp_car(cur)))
        {
          printf("Not good");
        }
      else
        {
          if (strcmp(lisp_symbol(lisp_car(cur)), "title") == 0)
            {
              if(( s = lisp_string(lisp_car(lisp_cdr(cur)))) != NULL)
                {
                  title = s;
                }
            }
          else if (strcmp(lisp_symbol(lisp_car(cur)), "description") == 0)
            {
              if(( s = lisp_string(lisp_car(lisp_cdr(cur)))) != NULL)
                {
                  description = s;
                }
            }
        }
      cursor = lisp_cdr (cursor);
    }
}

void st_subset::load(char *subset)
{
  FILE* fi;
  char filename[1024];
  char str[1024];
  int i;
  lisp_object_t* root_obj = 0;

  name = subset;

  snprintf(filename, 1024, "%s/levels/%s/info", st_dir, subset);
  if(!faccessible(filename))
    snprintf(filename, 1024, "%s/levels/%s/info", datadir.c_str(), subset);
  if(faccessible(filename))
    {
      fi = fopen(filename, "r");
      if (fi == NULL)
        {
          perror(filename);
        }
      lisp_stream_t stream;
      lisp_stream_init_file (&stream, fi);
      root_obj = lisp_read (&stream);

      if (root_obj->type == LISP_TYPE_EOF || root_obj->type == LISP_TYPE_PARSE_ERROR)
        {
          printf("World: Parse Error in file %s", filename);
        }

      lisp_object_t* cur = lisp_car(root_obj);

      if (!lisp_symbol_p (cur))
        {
          printf("World: Read error in %s",filename);
        }

      if (strcmp(lisp_symbol(cur), "supertux-level-subset") == 0)
        {
          parse(lisp_cdr(root_obj));

        }

      fclose(fi);

      snprintf(str, 1024, "%s.png", filename);
      if(faccessible(str))
        {
          texture_load(&image,str,IGNORE_ALPHA);
        }
      else
        {
          snprintf(filename, 1024, "%s/images/status/level-subset-info.png", datadir.c_str());
          texture_load(&image,filename,IGNORE_ALPHA);
        }
    }

  for(i=1; i != -1; ++i)
    {
      /* Get the number of levels in this subset */
      snprintf(filename, 1024, "%s/levels/%s/level%d.stl", st_dir, subset,i);
      if(!faccessible(filename))
        {
          snprintf(filename, 1024, "%s/levels/%s/level%d.stl", datadir.c_str(), subset,i);
          if(!faccessible(filename))
            break;
        }
    }
  levels = --i;
}

void st_subset::save()
{
  FILE* fi;
  string filename;

  /* Save data file: */
  filename = "/levels/" + name + "/";

  fcreatedir(filename.c_str());
  filename = string(st_dir) + "/levels/" + name + "/info";
  if(!fwriteable(filename.c_str()))
    filename = datadir + "/levels/" + name + "/info";
  if(fwriteable(filename.c_str()))
    {
      fi = fopen(filename.c_str(), "w");
      if (fi == NULL)
        {
          perror(filename.c_str());
        }

      /* Write header: */
      fprintf(fi,";SuperTux-Level-Subset\n");
      fprintf(fi,"(supertux-level-subset\n");

      /* Save title info: */
      fprintf(fi,"  (title \"%s\")\n", title.c_str());

      /* Save the description: */
      fprintf(fi,"  (description \"%s\")\n", description.c_str());

      fprintf( fi,")");
      fclose(fi);

    }
}

void st_subset::free()
{
  title.clear();
  description.clear();
  name.clear();
  texture_free(&image);
  levels = 0;
}

void level_default(st_level* plevel)
{
  int i,y;
  plevel->name = "UnNamed";
  plevel->theme = "antarctica";
  plevel->song_title = "Mortimers_chipdisko.mod";
  plevel->bkgd_image = "arctis.png";
  plevel->width = 21;
  plevel->time_left = 100;
  plevel->gravity = 10.;
  plevel->bkgd_red = 0;
  plevel->bkgd_green = 0;
  plevel->bkgd_blue = 0;

  for(i = 0; i < 15; ++i)
    {
      plevel->ia_tiles[i] = (unsigned int*) malloc((plevel->width+1)*sizeof(unsigned int));
      plevel->ia_tiles[i][plevel->width] = (unsigned int) '\0';
      for(y = 0; y < plevel->width; ++y)
        plevel->ia_tiles[i][y] = (unsigned int) '.';
      plevel->ia_tiles[i][plevel->width] = (unsigned int) '\0';

      plevel->bg_tiles[i] = (unsigned int*) malloc((plevel->width+1)*sizeof(unsigned int));
      plevel->bg_tiles[i][plevel->width] = (unsigned int) '\0';
      for(y = 0; y < plevel->width; ++y)
        plevel->bg_tiles[i][y] = (unsigned int) '.';
      plevel->bg_tiles[i][plevel->width] = (unsigned int) '\0';

      plevel->fg_tiles[i] = (unsigned int*) malloc((plevel->width+1)*sizeof(unsigned int));
      plevel->fg_tiles[i][plevel->width] = (unsigned int) '\0';
      for(y = 0; y < plevel->width; ++y)
        plevel->fg_tiles[i][y] = (unsigned int) '.';
      plevel->fg_tiles[i][plevel->width] = (unsigned int) '\0';

      plevel->dn_tiles[i] = (unsigned int*) malloc((plevel->width+1)*sizeof(unsigned int));
      plevel->dn_tiles[i][plevel->width] = (unsigned int) '\0';
      for(y = 0; y < plevel->width; ++y)
        plevel->dn_tiles[i][y] = (unsigned int) '.';
      plevel->dn_tiles[i][plevel->width] = (unsigned int) '\0';
    }
}

/* Load data for this level: */
/* Returns -1, if the loading of the level failed. */
int level_load(st_level* plevel, const  char *subset, int level)
{
  char filename[1024];

  /* Load data file: */

  snprintf(filename, 1024, "%s/levels/%s/level%d.stl", st_dir, subset, level);
  if(!faccessible(filename))
    snprintf(filename, 1024, "%s/levels/%s/level%d.stl", datadir.c_str(), subset, level);

  return level_load(plevel, filename);
}

int level_load(st_level* plevel, const char* filename)
{
  FILE * fi;
  lisp_object_t* root_obj = 0;
  fi = fopen(filename, "r");
  if (fi == NULL)
    {
      perror(filename);
      return -1;
    }

  lisp_stream_t stream;
  lisp_stream_init_file (&stream, fi);
  root_obj = lisp_read (&stream);

  if (root_obj->type == LISP_TYPE_EOF || root_obj->type == LISP_TYPE_PARSE_ERROR)
    {
      printf("World: Parse Error in file %s", filename);
    }

  vector<int> ia_tm;
  vector<int> bg_tm;
  vector<int> fg_tm;

  int version = 0;
  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-level") == 0)
    {
      LispReader reader(lisp_cdr(root_obj));

      reader.read_int("version",  &version);
      reader.read_int("width",  &plevel->width);
      reader.read_int("time",  &plevel->time_left);
      reader.read_int("bkgd_red",  &plevel->bkgd_red);
      reader.read_int("bkgd_green",  &plevel->bkgd_green);
      reader.read_int("bkgd_blue",  &plevel->bkgd_blue);
      reader.read_float("gravity",  &plevel->gravity);
      reader.read_string("name",  &plevel->name);
      reader.read_string("theme",  &plevel->theme);
      reader.read_string("music",  &plevel->song_title);
      reader.read_string("background",  &plevel->bkgd_image);
      reader.read_string("particle_system", &plevel->particle_system);
      reader.read_int_vector("background-tm",  &bg_tm);

      if (!reader.read_int_vector("interactive-tm", &ia_tm))
        reader.read_int_vector("tilemap", &ia_tm);

      reader.read_int_vector("foreground-tm",  &fg_tm);

      {
        lisp_object_t* cur = 0;
        if (reader.read_lisp("objects",  &cur))
          {
            while (!lisp_nil_p(cur))
              {
                lisp_object_t* data = lisp_car(cur);

                BadGuyData bg_data;
                bg_data.kind = badguykind_from_string(lisp_symbol(lisp_car(data)));
                LispReader reader(lisp_cdr(data));
                reader.read_int("x", &bg_data.x);
                reader.read_int("y", &bg_data.y);

                plevel->badguy_data.push_back(bg_data);
 
                cur = lisp_cdr(cur);
              }
          }        
      }

      // Convert old levels to the new tile numbers
      if (version == 0)
        {
          std::map<char, int> transtable;
          transtable['.'] = 0;
          transtable['x'] = 104;
          transtable['X'] = 77;
          transtable['y'] = 78;
          transtable['Y'] = 105;
          transtable['A'] = 83;
          transtable['B'] = 102;
          transtable['!'] = 103;
          transtable['a'] = 84;
          transtable['C'] = 85;
          transtable['D'] = 86;
          transtable['E'] = 87;
          transtable['F'] = 88;
          transtable['c'] = 89;
          transtable['d'] = 90;
          transtable['e'] = 91;
          transtable['f'] = 92;

          transtable['G'] = 93;
          transtable['H'] = 94;
          transtable['I'] = 95;
          transtable['J'] = 96;

          transtable['g'] = 97;
          transtable['h'] = 98;
          transtable['i'] = 99;
          transtable['j'] = 100
            ;
          transtable['#'] = 11;
          transtable['['] = 13; 
          transtable['='] = 14;
          transtable[']'] = 15;
          transtable['$'] = 82;
          transtable['^'] = 76;
          transtable['*'] = 80;
          transtable['|'] = 79;
          transtable['\\'] = 81;
          transtable['&'] = 75;

          int x = 0;
          int y = 0;
          for(std::vector<int>::iterator i = ia_tm.begin(); i != ia_tm.end(); ++i)
            {
              if (*i == '0' || *i == '1' || *i == '2')
                {
                  plevel->badguy_data.push_back(BadGuyData(static_cast<BadGuyKind>(*i-'0'),
                                                           x*32, y*32));
                  *i = 0;
                }
              else
                {
                  std::map<char, int>::iterator j = transtable.find(*i);
                  if (j != transtable.end())
                    *i = j->second;
                  else
                    printf("Error: conversion will fail, unsupported char: '%c' (%d)\n", *i, *i);
                }
              ++x;
              if (x >= plevel->width) {
                x = 0;
                ++y;
              }
            }
        }
    }

  for(int i = 0; i < 15; ++i)
    {
      plevel->dn_tiles[i] = (unsigned int*) calloc((plevel->width +1) , sizeof(unsigned int) );
      plevel->ia_tiles[i] = (unsigned int*) calloc((plevel->width +1) , sizeof(unsigned int) );
      plevel->bg_tiles[i] = (unsigned int*) calloc((plevel->width +1) , sizeof(unsigned int) );
      plevel->fg_tiles[i] = (unsigned int*) calloc((plevel->width +1) , sizeof(unsigned int) );
    }

  int i = 0;
  int j = 0;
  for(vector<int>::iterator it = ia_tm.begin(); it != ia_tm.end(); ++it, ++i)
    {
      plevel->ia_tiles[j][i] = (*it);
      if(i == plevel->width - 1)
        {
          i = -1;
          ++j;
        }
    }

  i = j = 0;
  for(vector<int>::iterator it = bg_tm.begin(); it != bg_tm.end(); ++it, ++i)
    {

      plevel->bg_tiles[j][i] = (*it);
      if(i == plevel->width - 1)
        {
          i = -1;
          ++j;
        }
    }

  i = j = 0;
  for(vector<int>::iterator it = fg_tm.begin(); it != fg_tm.end(); ++it, ++i)
    {

      plevel->fg_tiles[j][i] = (*it);
      if(i == plevel->width - 1)
        {
          i = -1;
          ++j;
        }
    }

  /* Set the global gravity to the latest loaded level's gravity */
  gravity = plevel->gravity;

  //  Mark the end position of this level!
  // FIXME: -10 is a rather random value, we still need some kind of
  // real levelend gola
  endpos = 32*(plevel->width-10);

  fclose(fi);
  return 0;
}

/* Save data for level: */

void level_save(st_level* plevel,const  char * subset, int level)
{
  FILE * fi;
  char filename[1024];
  int y,i;
  char str[80];

  /* Save data file: */
  sprintf(str, "/levels/%s/", subset);
  fcreatedir(str);
  snprintf(filename, 1024, "%s/levels/%s/level%d.stl", st_dir, subset, level);
  if(!fwriteable(filename))
    snprintf(filename, 1024, "%s/levels/%s/level%d.stl", datadir.c_str(), subset, level);

  fi = fopen(filename, "w");
  if (fi == NULL)
    {
      perror(filename);
      st_shutdown();
      exit(-1);
    }


  /* Write header: */
  fprintf(fi,";SuperTux-Level\n");
  fprintf(fi,"(supertux-level\n");

  fprintf(fi,"  (version %d)\n", 1);
  fprintf(fi,"  (name \"%s\")\n", plevel->name.c_str());
  fprintf(fi,"  (theme \"%s\")\n", plevel->theme.c_str());
  fprintf(fi,"  (music \"%s\")\n", plevel->song_title.c_str());
  fprintf(fi,"  (background \"%s\")\n", plevel->bkgd_image.c_str());
  fprintf(fi,"  (particle_system \"%s\")\n", plevel->particle_system.c_str());
  fprintf(fi,"  (bkgd_red %d)\n", plevel->bkgd_red);
  fprintf(fi,"  (bkgd_green %d)\n", plevel->bkgd_green);
  fprintf(fi,"  (bkgd_blue %d)\n", plevel->bkgd_blue);
  fprintf(fi,"  (time %d)\n", plevel->time_left);
  fprintf(fi,"  (width %d)\n", plevel->width);
  fprintf(fi,"  (gravity %2.1f)\n", plevel->gravity);
  fprintf(fi,"  (background-tm ");

  for(y = 0; y < 15; ++y)
    {
      for(i = 0; i < plevel->width; ++i)
        fprintf(fi," %d ", plevel->bg_tiles[y][i]);
    }

  fprintf( fi,")\n");
  fprintf(fi,"  (interactive-tm ");

  for(y = 0; y < 15; ++y)
    {
      for(i = 0; i < plevel->width; ++i)
        fprintf(fi," %d ", plevel->ia_tiles[y][i]);
    }

  fprintf( fi,")\n");
  fprintf(fi,"  (dynamic-tm ");

  for(y = 0; y < 15; ++y)
    {
      for(i = 0; i < plevel->width; ++i)
        fprintf(fi," %d ", plevel->dn_tiles[y][i]);
    }

  fprintf( fi,")\n");
  fprintf(fi,"  (foreground-tm ");

  for(y = 0; y < 15; ++y)
    {
      for(i = 0; i < plevel->width; ++i)
        fprintf(fi," %d ", plevel->fg_tiles[y][i]);
    }

  fprintf( fi,")");
  fprintf( fi,")\n");

  fclose(fi);
}


/* Unload data for this level: */

void level_free(st_level* plevel)
{
  int i;
  for(i=0; i < 15; ++i)
    free(plevel->bg_tiles[i]);
  for(i=0; i < 15; ++i)
    free(plevel->ia_tiles[i]);
  for(i=0; i < 15; ++i)
    free(plevel->dn_tiles[i]);
  for(i=0; i < 15; ++i)
    free(plevel->fg_tiles[i]);

  plevel->name.clear();
  plevel->theme.clear();
  plevel->song_title.clear();
  plevel->bkgd_image.clear();

  plevel->badguy_data.clear();
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

  if(!plevel->bkgd_image.empty())
    {
      char fname[1024];
      snprintf(fname, 1024, "%s/background/%s", st_dir, plevel->bkgd_image.c_str());
      if(!faccessible(fname))
        snprintf(fname, 1024, "%s/images/background/%s", datadir.c_str(), plevel->bkgd_image.c_str());
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

void level_load_image(texture_type* ptexture, string theme,const  char * file, int use_alpha)
{
  char fname[1024];

  snprintf(fname, 1024, "%s/themes/%s/%s", st_dir, theme.c_str(), file);
  if(!faccessible(fname))
    snprintf(fname, 1024, "%s/images/themes/%s/%s", datadir.c_str(), theme.c_str(), file);

  texture_load(ptexture, fname, use_alpha);
}

void tilemap_change_size(unsigned int** tilemap[15], int w, int old_w)
{
  int j,y;
  for(y = 0; y < 15; ++y)
    {
      *tilemap[y] = (unsigned int*) realloc(*tilemap[y],(w+1)*sizeof(unsigned int));
      if(w > old_w)
        for(j = 0; j < w - old_w; ++j)
          *tilemap[y][old_w+j] = 0;
      *tilemap[y][w] = 0;
    }
}

/* Change the size of a level (width) */
void level_change_size (st_level* plevel, int new_width)
{
  if(new_width < 21)
    new_width = 21;
  tilemap_change_size((unsigned int***)&plevel->ia_tiles,new_width,plevel->width);
  tilemap_change_size((unsigned int***)&plevel->dn_tiles,new_width,plevel->width);
  tilemap_change_size((unsigned int***)&plevel->bg_tiles,new_width,plevel->width);
  tilemap_change_size((unsigned int***)&plevel->fg_tiles,new_width,plevel->width);
  plevel->width = new_width;

}

/* Edit a piece of the map! */

void level_change(st_level* plevel, float x, float y, int tm, unsigned int c)
{
  int xx, yy;

  yy = ((int)y / 32);
  xx = ((int)x / 32);

  if (yy >= 0 && yy < 15 && xx >= 0 && xx <= plevel->width)
    {
      switch(tm)
        {
        case 0:
          plevel->bg_tiles[yy][xx] = c;
        case 1:
          plevel->ia_tiles[yy][xx] = c;
        case 2:
          plevel->dn_tiles[yy][xx] = c;
        case 4:
          plevel->fg_tiles[yy][xx] = c;
        }
    }
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

  level_song = load_song(datadir + "/music/" + plevel->song_title);

  song_path = (char *) malloc(sizeof(char) * datadir.length() +
                              strlen(plevel->song_title.c_str()) + 8 + 5);
  song_subtitle = strdup(plevel->song_title.c_str());
  strcpy(strstr(song_subtitle, "."), "\0");
  sprintf(song_path, "%s/music/%s-fast%s", datadir.c_str(), song_subtitle, strstr(plevel->song_title.c_str(), "."));
  level_song_fast = load_song(song_path);
  free(song_subtitle);
  free(song_path);
}
