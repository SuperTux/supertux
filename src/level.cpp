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
#include "tile.h"
#include "lispreader.h"

using namespace std;

st_subset::st_subset()
{
  levels = 0;
}

void st_subset::create(const std::string& subset_name)
{
  Level new_lev;
  st_subset new_subset;
  new_subset.name = subset_name;
  new_subset.title = "Unknown Title";
  new_subset.description = "No description so far.";
  new_subset.save();
  new_lev.init_defaults();
  new_lev.save(subset_name.c_str(),1);
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
          image = new Surface(str,IGNORE_ALPHA);
        }
      else
        {
          snprintf(filename, 1024, "%s/images/status/level-subset-info.png", datadir.c_str());
          image = new Surface(filename,IGNORE_ALPHA);
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
  delete image;
  levels = 0;
}

void
Level::init_defaults()
{
  name       = "UnNamed";
  author     = "UnNamed";
  theme      = "antarctica";
  song_title = "Mortimers_chipdisko.mod";
  bkgd_image = "arctis.png";
  width      = 21;
  time_left  = 100;
  gravity    = 10.;
  bkgd_top_red   = 0;
  bkgd_top_green = 0;
  bkgd_top_blue  = 0;
  bkgd_bottom_red   = 0;
  bkgd_bottom_green = 0;
  bkgd_bottom_blue  = 0;
  endpos     = 0;

  for(int i = 0; i < 15; ++i)
    {
      ia_tiles[i] = (unsigned int*) malloc((width+1)*sizeof(unsigned int));
      ia_tiles[i][width] = (unsigned int) '\0';
      for(int y = 0; y < width; ++y)
        ia_tiles[i][y] = 0;
      ia_tiles[i][width] = (unsigned int) '\0';

      bg_tiles[i] = (unsigned int*) malloc((width+1)*sizeof(unsigned int));
      bg_tiles[i][width] = (unsigned int) '\0';
      for(int y = 0; y < width; ++y)
        bg_tiles[i][y] = 0;
      bg_tiles[i][width] = (unsigned int) '\0';

      fg_tiles[i] = (unsigned int*) malloc((width+1)*sizeof(unsigned int));
      fg_tiles[i][width] = (unsigned int) '\0';
      for(int y = 0; y < width; ++y)
        fg_tiles[i][y] = 0;
      fg_tiles[i][width] = (unsigned int) '\0';
    }
}

int
Level::load(const std::string& subset, int level)
{
  char filename[1024];

  // Load data file:
  snprintf(filename, 1024, "%s/levels/%s/level%d.stl", st_dir, subset.c_str(), level);
  if(!faccessible(filename))
    snprintf(filename, 1024, "%s/levels/%s/level%d.stl", datadir.c_str(), subset.c_str(), level);

  return load(filename);
}

int 
Level::load(const std::string& filename)
{
  FILE * fi;
  lisp_object_t* root_obj = 0;
  fi = fopen(filename.c_str(), "r");
  if (fi == NULL)
    {
      perror(filename.c_str());
      return -1;
    }

  lisp_stream_t stream;
  lisp_stream_init_file (&stream, fi);
  root_obj = lisp_read (&stream);

  if (root_obj->type == LISP_TYPE_EOF || root_obj->type == LISP_TYPE_PARSE_ERROR)
    {
      printf("World: Parse Error in file %s", filename.c_str());
    }

  vector<int> ia_tm;
  vector<int> bg_tm;
  vector<int> fg_tm;

  int version = 0;
  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-level") == 0)
    {
      LispReader reader(lisp_cdr(root_obj));

      reader.read_int("version",  &version);
      reader.read_int("width",  &width);
      reader.read_int("time",  &time_left);
      reader.read_int("bkgd_top_red",  &bkgd_top_red);
      reader.read_int("bkgd_top_green",  &bkgd_top_green);
      reader.read_int("bkgd_top_blue",  &bkgd_top_blue);
      reader.read_int("bkgd_bottom_red",  &bkgd_bottom_red);
      reader.read_int("bkgd_bottom_green",  &bkgd_bottom_green);
      reader.read_int("bkgd_bottom_blue",  &bkgd_bottom_blue);
      reader.read_float("gravity",  &gravity);
      reader.read_string("name",  &name);
      reader.read_string("author", &author);
      reader.read_string("theme",  &theme);
      reader.read_string("music",  &song_title);
      reader.read_string("background",  &bkgd_image);
      reader.read_string("particle_system", &particle_system);
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

                badguy_data.push_back(bg_data);

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
                  badguy_data.push_back(BadGuyData(static_cast<BadGuyKind>(*i-'0'),
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
              if (x >= width)
                {
                  x = 0;
                  ++y;
                }
            }
        }
    }

  for(int i = 0; i < 15; ++i)
    {
      ia_tiles[i] = (unsigned int*) calloc((width +1) , sizeof(unsigned int) );
      bg_tiles[i] = (unsigned int*) calloc((width +1) , sizeof(unsigned int) );
      fg_tiles[i] = (unsigned int*) calloc((width +1) , sizeof(unsigned int) );
    }

  int i = 0;
  int j = 0;
  for(vector<int>::iterator it = ia_tm.begin(); it != ia_tm.end(); ++it, ++i)
    {
      ia_tiles[j][i] = (*it);
      if(i == width - 1)
        {
          i = -1;
          ++j;
        }
    }

  i = j = 0;
  for(vector<int>::iterator it = bg_tm.begin(); it != bg_tm.end(); ++it, ++i)
    {

      bg_tiles[j][i] = (*it);
      if(i == width - 1)
        {
          i = -1;
          ++j;
        }
    }

  i = j = 0;
  for(vector<int>::iterator it = fg_tm.begin(); it != fg_tm.end(); ++it, ++i)
    {

      fg_tiles[j][i] = (*it);
      if(i == width - 1)
        {
          i = -1;
          ++j;
        }
    }

  //  Mark the end position of this level!
  // FIXME: -10 is a rather random value, we still need some kind of
  // real levelend gola
  endpos = 32*(width-10);

  fclose(fi);
  return 0;
}

/* Save data for level: */

void 
Level::save(const  char * subset, int level)
{
  char filename[1024];
  char str[80];

  /* Save data file: */
  sprintf(str, "/levels/%s/", subset);
  fcreatedir(str);
  snprintf(filename, 1024, "%s/levels/%s/level%d.stl", st_dir, subset, level);
  if(!fwriteable(filename))
    snprintf(filename, 1024, "%s/levels/%s/level%d.stl", datadir.c_str(), subset, level);

  FILE * fi = fopen(filename, "w");
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
  fprintf(fi,"  (name \"%s\")\n", name.c_str());
  fprintf(fi,"  (author \"%s\")\n", author.c_str());
  fprintf(fi,"  (theme \"%s\")\n", theme.c_str());
  fprintf(fi,"  (music \"%s\")\n", song_title.c_str());
  fprintf(fi,"  (background \"%s\")\n", bkgd_image.c_str());
  fprintf(fi,"  (particle_system \"%s\")\n", particle_system.c_str());
  fprintf(fi,"  (bkgd_top_red %d)\n", bkgd_top_red);
  fprintf(fi,"  (bkgd_top_green %d)\n", bkgd_top_green);
  fprintf(fi,"  (bkgd_top_blue %d)\n", bkgd_top_blue);
  fprintf(fi,"  (bkgd_bottom_red %d)\n", bkgd_bottom_red);
  fprintf(fi,"  (bkgd_bottom_green %d)\n", bkgd_bottom_green);
  fprintf(fi,"  (bkgd_bottom_blue %d)\n", bkgd_bottom_blue);
  fprintf(fi,"  (time %d)\n", time_left);
  fprintf(fi,"  (width %d)\n", width);
  fprintf(fi,"  (gravity %2.1f)\n", gravity);
  fprintf(fi,"  (background-tm ");

  for(int y = 0; y < 15; ++y)
    {
      for(int i = 0; i < width; ++i)
        fprintf(fi," %d ", bg_tiles[y][i]);
    }

  fprintf( fi,")\n");
  fprintf(fi,"  (interactive-tm ");

  for(int y = 0; y < 15; ++y)
    {
      for(int i = 0; i < width; ++i)
        fprintf(fi," %d ", ia_tiles[y][i]);
    }

  fprintf( fi,")\n");
  fprintf(fi,"  (foreground-tm ");

  for(int y = 0; y < 15; ++y)
    {
      for(int i = 0; i < width; ++i)
        fprintf(fi," %d ", fg_tiles[y][i]);
    }

  fprintf( fi,")\n");
  fprintf( fi,"(objects\n");

  for(std::vector<BadGuyData>::iterator it = badguy_data.begin();
      it != badguy_data.end();
      ++it)
    fprintf( fi,"(%s (x %d) (y %d))\n",badguykind_to_string((*it).kind).c_str(),(*it).x,(*it).y);

  fprintf( fi,")\n");

  fprintf( fi,")\n");

  fclose(fi);
}


/* Unload data for this level: */

void
Level::cleanup()
{
  for(int i=0; i < 15; ++i)
    free(bg_tiles[i]);
  for(int i=0; i < 15; ++i)
    free(ia_tiles[i]);
  for(int i=0; i < 15; ++i)
    free(fg_tiles[i]);

  name.clear();
  author.clear();
  theme.clear();
  song_title.clear();
  bkgd_image.clear();

  badguy_data.clear();
}

void 
Level::load_gfx()
{
  if(!bkgd_image.empty())
    {
      char fname[1024];
      snprintf(fname, 1024, "%s/background/%s", st_dir, bkgd_image.c_str());
      if(!faccessible(fname))
        snprintf(fname, 1024, "%s/images/background/%s", datadir.c_str(), bkgd_image.c_str());
      img_bkgd = new Surface(fname, IGNORE_ALPHA);
    }
  else
    {
      /* Quick hack to make sure an image is loaded, when we are freeing it afterwards. */
      load_image(&img_bkgd, theme,"solid0.png", IGNORE_ALPHA);
    }
}

void
Level::free_gfx()
{
  delete img_bkgd;
}

/* Load a level-specific graphic... */
void
Level::load_image(Surface** ptexture, string theme,const  char * file, int use_alpha)
{
  char fname[1024];

  snprintf(fname, 1024, "%s/themes/%s/%s", st_dir, theme.c_str(), file);
  if(!faccessible(fname))
    snprintf(fname, 1024, "%s/images/themes/%s/%s", datadir.c_str(), theme.c_str(), file);

  *ptexture = new Surface(fname, use_alpha);
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
void 
Level::change_size (int new_width)
{
  if(new_width < 21)
    new_width = 21;

  tilemap_change_size((unsigned int***)&ia_tiles, new_width, width);
  tilemap_change_size((unsigned int***)&bg_tiles, new_width, width);
  tilemap_change_size((unsigned int***)&fg_tiles, new_width, width);

  width = new_width;
}

void
Level::change(float x, float y, int tm, unsigned int c)
{
  int yy = ((int)y / 32);
  int xx = ((int)x / 32);

  if (yy >= 0 && yy < 15 && xx >= 0 && xx <= width)
    {
      switch(tm)
        {
        case TM_BG:
          bg_tiles[yy][xx] = c;
          break;
        case TM_IA:
          ia_tiles[yy][xx] = c;
          break;
        case TM_FG:
          fg_tiles[yy][xx] = c;
          break;
        }
    }
}

void 
Level::free_song(void)
{
  free_music(level_song);
  free_music(level_song_fast);
}

void
Level::load_song()
{
  char* song_path;
  char* song_subtitle;

  level_song = ::load_song(datadir + "/music/" + song_title);

  song_path = (char *) malloc(sizeof(char) * datadir.length() +
                              strlen(song_title.c_str()) + 8 + 5);
  song_subtitle = strdup(song_title.c_str());
  strcpy(strstr(song_subtitle, "."), "\0");
  sprintf(song_path, "%s/music/%s-fast%s", datadir.c_str(), 
          song_subtitle, strstr(song_title.c_str(), "."));
  level_song_fast = ::load_song(song_path);
  free(song_subtitle);
  free(song_path);
}


unsigned int 
Level::gettileid(float x, float y)
{
  int xx, yy;
  unsigned int c;

  yy = ((int)y / 32);
  xx = ((int)x / 32);

  if (yy >= 0 && yy < 15 && xx >= 0 && xx <= width)
    c = ia_tiles[yy][xx];
  else
    c = 0;

  return c;
}

/* EOF */
