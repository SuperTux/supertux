//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 SuperTux Development Team, see AUTHORS for details
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

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
#include "resources.h"
#ifndef NOSOUND
#include "music_manager.h"
#endif

using namespace std;

LevelSubset::LevelSubset()
    : image(0), levels(0)
{
}

LevelSubset::~LevelSubset()
{
  delete image;
}

void LevelSubset::create(const std::string& subset_name)
{
  Level new_lev;
  LevelSubset new_subset;
  new_subset.name = subset_name;
  new_subset.title = "Unknown Title";
  new_subset.description = "No description so far.";
  new_subset.save();
  new_lev.init_defaults();
  new_lev.save(subset_name, 1);
}

void LevelSubset::parse (lisp_object_t* cursor)
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

void LevelSubset::load(char *subset)
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

      lisp_free(root_obj);
      fclose(fi);

      snprintf(str, 1024, "%s.png", filename);
      if(faccessible(str))
        {
          delete image;
          image = new Surface(str,IGNORE_ALPHA);
        }
      else
        {
          snprintf(filename, 1024, "%s/images/status/level-subset-info.png", datadir.c_str());
          delete image;
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

void LevelSubset::save()
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

Level::Level()
  : img_bkgd(0)
{
  init_defaults();
}

Level::Level(const std::string& subset, int level)
  : img_bkgd(0)
{
  if(load(subset, level) < 0)
    st_abort("Couldn't load level from subset", subset.c_str());
}

Level::Level(const std::string& filename)
  : img_bkgd(0)
{
  if(load(filename) < 0)
    st_abort("Couldn't load level " , filename.c_str());
}

Level::~Level()
{
  delete img_bkgd;
}

void
Level::init_defaults()
{
  name       = "UnNamed";
  author     = "UnNamed";
  song_title = "Mortimers_chipdisko.mod";
  bkgd_image = "arctis.png";
  width      = 21;
  start_pos_x = 100;
  start_pos_y = 170;
  time_left  = 100;
  gravity    = 10.;
  back_scrolling = false;
  hor_autoscroll_speed = 0;
  bkgd_speed = 50;
  bkgd_top.red   = 0;
  bkgd_top.green = 0;
  bkgd_top.blue  = 0;
  bkgd_bottom.red   = 255;
  bkgd_bottom.green = 255;
  bkgd_bottom.blue  = 255;

  for(int i = 0; i < 15; ++i)
    {
      ia_tiles[i].resize(width+1, 0);
      ia_tiles[i][width] = (unsigned int) '\0';

      for(int y = 0; y < width; ++y)
        ia_tiles[i][y] = 0;

      bg_tiles[i].resize(width+1, 0);
      bg_tiles[i][width] = (unsigned int) '\0';
      for(int y = 0; y < width; ++y)
        bg_tiles[i][y] = 0;

      fg_tiles[i].resize(width+1, 0);
      fg_tiles[i][width] = (unsigned int) '\0';
      for(int y = 0; y < width; ++y)
        fg_tiles[i][y] = 0;
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
  lisp_object_t* root_obj = lisp_read_from_file(filename);
  if (!root_obj)
    {
      std::cout << "Level: Couldn't load file: " << filename << std::endl;
      return -1;
    }

  if (root_obj->type == LISP_TYPE_EOF || root_obj->type == LISP_TYPE_PARSE_ERROR)
    {
      printf("World: Parse Error in file %s", filename.c_str());
      return -1;
    }

  vector<int> ia_tm;
  vector<int> bg_tm;
  vector<int> fg_tm;

  int version = 0;
  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-level") == 0)
    {
      LispReader reader(lisp_cdr(root_obj));
      version = 0;
      reader.read_int("version",  &version);
      if(!reader.read_int("width",  &width))
        st_abort("No width specified for level.", "");
      if (!reader.read_int("start_pos_x", &start_pos_x)) start_pos_x = 100;
      if (!reader.read_int("start_pos_y", &start_pos_y)) start_pos_y = 170;
      time_left = 500;
      if(!reader.read_int("time",  &time_left)) {
        printf("Warning no time specified for level.\n");
      }
      
      back_scrolling = false;
      reader.read_bool("back_scrolling",  &back_scrolling);

      hor_autoscroll_speed = 0;
      reader.read_float("hor_autoscroll_speed",  &hor_autoscroll_speed);
      
      bkgd_speed = 50;
      reader.read_int("bkgd_speed",  &bkgd_speed);

      
      bkgd_top.red = bkgd_top.green = bkgd_top.blue = 0;
      reader.read_int("bkgd_red_top",  &bkgd_top.red);
      reader.read_int("bkgd_green_top",  &bkgd_top.green);
      reader.read_int("bkgd_blue_top",  &bkgd_top.blue);

      bkgd_bottom.red = bkgd_bottom.green = bkgd_bottom.blue = 0;
      reader.read_int("bkgd_red_bottom",  &bkgd_bottom.red);
      reader.read_int("bkgd_green_bottom",  &bkgd_bottom.green);
      reader.read_int("bkgd_blue_bottom",  &bkgd_bottom.blue);

      gravity = 10;
      reader.read_float("gravity",  &gravity);
      name = "Noname";
      reader.read_string("name",  &name);
      author = "unknown author";
      reader.read_string("author", &author);
      song_title = "";
      reader.read_string("music",  &song_title);
      bkgd_image = "";
      reader.read_string("background",  &bkgd_image);
      particle_system = "";
      reader.read_string("particle_system", &particle_system);

      reader.read_int_vector("background-tm",  &bg_tm);

      if (!reader.read_int_vector("interactive-tm", &ia_tm))
        reader.read_int_vector("tilemap", &ia_tm);

      reader.read_int_vector("foreground-tm",  &fg_tm);

      { // Read ResetPoints
        lisp_object_t* cur = 0;
        if (reader.read_lisp("reset-points",  &cur))
          {
            while (!lisp_nil_p(cur))
              {
                lisp_object_t* data = lisp_car(cur);

                ResetPoint pos;

                LispReader reader(lisp_cdr(data));
                if (reader.read_int("x", &pos.x)
                    && reader.read_int("y", &pos.y))
                  {
                    reset_points.push_back(pos);
                  }

                cur = lisp_cdr(cur);
              }
          }
      }

      { // Read BadGuys
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
                reader.read_bool("stay-on-platform", &bg_data.stay_on_platform);

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
                                                   x*32, y*32, false));
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
      ia_tiles[i].resize(width + 1, 0);
      bg_tiles[i].resize(width + 1, 0);
      fg_tiles[i].resize(width + 1, 0);
    }

  int i = 0;
  int j = 0;
  for(vector<int>::iterator it = ia_tm.begin(); it != ia_tm.end(); ++it, ++i)
    {
      if(j >= 15)
        {
        std::cerr << "Warning: Level higher than 15 interactive tiles."
                     "Ignoring by cutting tiles.\n"
                     "The level might not be finishable anymore!\n";
        break;
        }

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
      if(j >= 15)
        {
        std::cerr << "Warning: Level higher than 15 background tiles."
                     "Ignoring by cutting tiles.\n";
        break;
        }

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
      if(j >= 15)
        {
        std::cerr << "Warning: Level higher than 15 foreground tiles."
                     "Ignoring by cutting tiles.\n";
        break;
        }

      fg_tiles[j][i] = (*it);
      if(i == width - 1)
        {
          i = -1;
          ++j;
        }
    }

  lisp_free(root_obj);
  return 0;
}

/* Save data for level: */

void 
Level::save(const std::string& subset, int level)
{
  char filename[1024];
  char str[80];

  /* Save data file: */
  sprintf(str, "/levels/%s/", subset.c_str());
  fcreatedir(str);
  snprintf(filename, 1024, "%s/levels/%s/level%d.stl", st_dir, subset.c_str(),
      level);
  if(!fwriteable(filename))
    snprintf(filename, 1024, "%s/levels/%s/level%d.stl", datadir.c_str(),
        subset.c_str(), level);

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
  fprintf(fi,"  (music \"%s\")\n", song_title.c_str());
  fprintf(fi,"  (background \"%s\")\n", bkgd_image.c_str());
  fprintf(fi,"  (particle_system \"%s\")\n", particle_system.c_str());
  fprintf(fi,"  (bkgd_speed %d)\n", bkgd_speed);
  fprintf(fi,"  (bkgd_red_top %d)\n", bkgd_top.red);
  fprintf(fi,"  (bkgd_green_top %d)\n", bkgd_top.green);
  fprintf(fi,"  (bkgd_blue_top %d)\n", bkgd_top.blue);
  fprintf(fi,"  (bkgd_red_bottom %d)\n", bkgd_bottom.red);
  fprintf(fi,"  (bkgd_green_bottom %d)\n", bkgd_bottom.green);
  fprintf(fi,"  (bkgd_blue_bottom %d)\n", bkgd_bottom.blue);
  fprintf(fi,"  (time %d)\n", time_left);
  fprintf(fi,"  (width %d)\n", width);
  if(back_scrolling)
    fprintf(fi,"  (back_scrolling #t)\n"); 
  else
    fprintf(fi,"  (back_scrolling #f)\n");
  fprintf(fi,"  (hor_autoscroll_speed %2.1f)\n", hor_autoscroll_speed);
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

  fprintf( fi,"(reset-points\n");
  for(std::vector<ResetPoint>::iterator i = reset_points.begin();
      i != reset_points.end(); ++i)
    fprintf( fi,"(point (x %d) (y %d))\n",i->x, i->y);
  fprintf( fi,")\n");

  fprintf( fi,"(objects\n");

  for(std::vector<BadGuyData>::iterator it = badguy_data.begin();
      it != badguy_data.end();
      ++it)
    fprintf( fi,"(%s (x %d) (y %d) (stay-on-platform %s))\n",
             badguykind_to_string((*it).kind).c_str(),(*it).x,(*it).y,
             it->stay_on_platform ? "#t" : "#f");

  fprintf( fi,")\n");

  fprintf( fi,")\n");

  fclose(fi);
}


/* Unload data for this level: */

void
Level::cleanup()
{
  for(int i=0; i < 15; ++i)
    {
      bg_tiles[i].clear();
      ia_tiles[i].clear();
      fg_tiles[i].clear();
    }

  reset_points.clear();
  name = "";
  author = "";
  song_title = "";
  bkgd_image = "";

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
      delete img_bkgd;
      img_bkgd = new Surface(fname, IGNORE_ALPHA);
    }
  else
    {
      delete img_bkgd;
      img_bkgd = 0;
    }
}

/* Load a level-specific graphic... */
void Level::load_image(Surface** ptexture, string theme,const  char * file, int use_alpha)
{
  char fname[1024];

  snprintf(fname, 1024, "%s/themes/%s/%s", st_dir, theme.c_str(), file);
  if(!faccessible(fname))
    snprintf(fname, 1024, "%s/images/themes/%s/%s", datadir.c_str(), theme.c_str(), file);

  *ptexture = new Surface(fname, use_alpha);
}

/* Change the size of a level (width) */
void 
Level::change_size (int new_width)
{
  if(new_width < 21)
    new_width = 21;

  for(int y = 0; y < 15; ++y)
    {
      ia_tiles[y].resize(new_width, 0);
      bg_tiles[y].resize(new_width, 0);
      fg_tiles[y].resize(new_width, 0);
    }

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

#ifndef NOSOUND
void
Level::load_song()
{
  char* song_path;
  char* song_subtitle;

  level_song = music_manager->load_music(datadir + "/music/" + song_title);

  song_path = (char *) malloc(sizeof(char) * datadir.length() +
                              strlen(song_title.c_str()) + 8 + 5);
  song_subtitle = strdup(song_title.c_str());
  strcpy(strstr(song_subtitle, "."), "\0");
  sprintf(song_path, "%s/music/%s-fast%s", datadir.c_str(), 
          song_subtitle, strstr(song_title.c_str(), "."));
  if(!music_manager->exists_music(song_path)) {
    level_song_fast = level_song;
  } else {
    level_song_fast = music_manager->load_music(song_path);
  }
  free(song_subtitle);
  free(song_path);
}

MusicRef
Level::get_level_music()
{
  return level_song;
}

MusicRef
Level::get_level_music_fast()
{
  return level_song_fast;
}
#endif

unsigned int 
Level::gettileid(float x, float y) const
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

unsigned int
Level::get_tile_at(int x, int y) const
{
  if(x < 0 || x > width || y < 0 || y > 14)
    return 0;
  
  return ia_tiles[y][x];
}

/* EOF */
