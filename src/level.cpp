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
#include <fstream>
#include "globals.h"
#include "setup.h"
#include "screen.h"
#include "level.h"
#include "physic.h"
#include "scene.h"
#include "tile.h"
#include "lispreader.h"
#include "resources.h"
#include "music_manager.h"
#include "gameobjs.h"
#include "world.h"
#include "lispwriter.h"

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
  new_lev.save(subset_name, 1, 0);
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
  bkgd_image = "arctis.jpg";
  width      = 0;
  height     = 0;
  start_pos.x = 100;
  start_pos.y = 170;
  time_left  = 100;
  gravity    = 10.;
  bkgd_speed = 50;
  bkgd_top.red   = 0;
  bkgd_top.green = 0;
  bkgd_top.blue  = 0;
  bkgd_bottom.red   = 255;
  bkgd_bottom.green = 255;
  bkgd_bottom.blue  = 255;

  resize(21, 19);
}

int
Level::load(const std::string& subset, int level, World* world)
{
  char filename[1024];

  // Load data file:
  snprintf(filename, 1024, "%s/levels/%s/level%d.stl", st_dir, subset.c_str(), level);
  if(!faccessible(filename))
    snprintf(filename, 1024, "%s/levels/%s/level%d.stl", datadir.c_str(), subset.c_str(), level);

  return load(filename, world);
}

int 
Level::load(const std::string& filename, World* world)
{
  lisp_object_t* root_obj = lisp_read_from_file(filename);
  if (!root_obj)
    {
      std::cout << "Level: Couldn't load file: " << filename << std::endl;
      return -1;
    }

  if (root_obj->type == LISP_TYPE_EOF || root_obj->type == LISP_TYPE_PARSE_ERROR)
    {
      lisp_free(root_obj);
      std::cout << "World: Parse Error in file '" << filename
                << "'.\n";
      return -1;
    }

  int version = 0;
  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-level") == 0)
    {
      LispReader reader(lisp_cdr(root_obj));
      version = 0;
      reader.read_int("version",  &version);
      if(!reader.read_int("width",  &width))
        st_abort("No width specified for level.", "");
      if (!reader.read_float("start_pos_x", &start_pos.x)) start_pos.x = 100;
      if (!reader.read_float("start_pos_y", &start_pos.y)) start_pos.y = 170;
      time_left = 500;
      if(!reader.read_int("time",  &time_left)) {
        printf("Warning: no time specified for level.\n");
      }
      
      height = 15;
      if(!reader.read_int("height",  &height)) {
        printf("Warning: no height specified for level.\n");
      }
      
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

      reader.read_int_vector("background-tm",  &bg_tiles);
      if(int(bg_tiles.size()) != width * height)
        st_abort("Wrong size of backgroundtilemap", "");

      if (!reader.read_int_vector("interactive-tm", &ia_tiles))
        reader.read_int_vector("tilemap", &ia_tiles);
      if(int(ia_tiles.size()) != width * height)
        st_abort("Wrong size of interactivetilemap", "");      

      reader.read_int_vector("foreground-tm",  &fg_tiles);
      if(int(fg_tiles.size()) != width * height)
        st_abort("Wrong size of foregroundtilemap", "");      

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

      { // Read Objects
        lisp_object_t* cur = 0;
        if (reader.read_lisp("objects",  &cur))
          {
            if(world)
              world->parse_objects(cur);
          }
      }

      { // Read Camera
        lisp_object_t* cur = 0;
        if (reader.read_lisp("camera", &cur))
          {
            LispReader reader(cur);
            if(world) {
              world->camera->read(reader);
            }
          }
      }
    }

  lisp_free(root_obj);
  return 0;
}

/* Save data for level: */

void 
Level::save(const std::string& subset, int level, World* world)
{
  char filename[1024];
  char str[80];

  /* Save data file: */
  snprintf(str, sizeof(str), "/levels/%s/", subset.c_str());
  fcreatedir(str);
  snprintf(filename, sizeof(filename),
      "%s/levels/%s/level%d.stl", st_dir, subset.c_str(), level);
  if(!fwriteable(filename))
    snprintf(filename, sizeof(filename), "%s/levels/%s/level%d.stl",
        datadir.c_str(), subset.c_str(), level);

  std::ofstream out(filename);
  if(!out.good()) {
    st_abort("Couldn't write file.", filename);
  }
  LispWriter writer(out);

  /* Write header: */
  writer.write_comment("SuperTux level made using the built-in leveleditor");
  writer.start_list("supertux-level");

  writer.write_int("version", 1);
  writer.write_string("name", name);
  writer.write_string("author", author);
  writer.write_string("music", song_title);
  writer.write_string("background", bkgd_image);
  writer.write_string("particle_system", particle_system);
  writer.write_int("bkgd_speed", bkgd_speed);
  writer.write_int("bkgd_red_top", bkgd_top.red);
  writer.write_int("bkgd_green_top", bkgd_top.green);
  writer.write_int("bkgd_blue_top", bkgd_top.blue);
  writer.write_int("bkgd_red_bottom", bkgd_bottom.red);
  writer.write_int("bkgd_green_bottom", bkgd_bottom.green);
  writer.write_int("bkgd_blue_bottom", bkgd_bottom.blue);
  writer.write_int("time", time_left);
  writer.write_int("width", width);
  writer.write_int("height", height);
  writer.write_float("gravity", gravity);

  writer.write_int_vector("background-tm", bg_tiles);
  writer.write_int_vector("interactive-tm", ia_tiles);
  writer.write_int_vector("foreground-tm", fg_tiles);

  writer.start_list("reset-points");
  for(std::vector<ResetPoint>::iterator i = reset_points.begin();
      i != reset_points.end(); ++i) {
    writer.start_list("point");
    writer.write_int("x", i->x);
    writer.write_int("y", i->y);
    writer.end_list("point");
  }
  writer.end_list("reset-points");

  // write objects
  writer.start_list("objects");
  // pick all objects that can be written into a levelfile
  for(std::vector<GameObject*>::iterator it = world->gameobjects.begin();
      it != world->gameobjects.end(); ++it) {
    Serializable* serializable = dynamic_cast<Serializable*> (*it);
    if(serializable)
      serializable->write(writer);
  }
  writer.end_list("objects");

  writer.end_list("supertux-level");
  out.close();
}

/* Unload data for this level: */
void
Level::cleanup()
{
  bg_tiles.clear();
  ia_tiles.clear();
  fg_tiles.clear();

  reset_points.clear();
  name = "";
  author = "";
  song_title = "";
  bkgd_image = "";
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

/* Change the size of a level */
void 
Level::resize(int new_width, int new_height)
{
  if(new_width < width) {
    // remap tiles for new width
    for(int y = 0; y < height && y < new_height; ++y) {
      for(int x = 0; x < new_width; ++x) {
        ia_tiles[y * new_width + x] = ia_tiles[y * width + x];
        bg_tiles[y * new_width + x] = bg_tiles[y * width + x];
        fg_tiles[y * new_width + x] = fg_tiles[y * width + x];
      }
    }
  }

  ia_tiles.resize(new_width * new_height);
  bg_tiles.resize(new_width * new_height);
  fg_tiles.resize(new_width * new_height); 

  if(new_width > width) {
    // remap tiles
    for(int y = std::min(height, new_height)-1; y >= 0; --y) {
      for(int x = new_width-1; x >= 0; --x) {
        if(x >= width) {
          ia_tiles[y * new_width + x] = 0;
          bg_tiles[y * new_width + x] = 0;
          fg_tiles[y * new_width + x] = 0;
        } else {
          ia_tiles[y * new_width + x] = ia_tiles[y * width + x];
          bg_tiles[y * new_width + x] = bg_tiles[y * width + x];
          fg_tiles[y * new_width + x] = fg_tiles[y * width + x];
        }
      }
    }
  }

  height = new_height;
  width = new_width;
}

void
Level::change(float x, float y, int tm, unsigned int c)
{
  int yy = ((int)y / 32);
  int xx = ((int)x / 32);

  if (yy >= 0 && yy < height && xx >= 0 && xx <= width)
    {
      switch(tm)
        {
        case TM_BG:
          bg_tiles[yy * width + xx] = c;
          break;
        case TM_IA:
          ia_tiles[yy * width + xx] = c;
          break;
        case TM_FG:
          fg_tiles[yy * width + xx] = c;
          break;
        }
    }
}

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

unsigned int 
Level::gettileid(float x, float y) const
{
  int xx, yy;
  unsigned int c;

  yy = ((int)y / 32);
  xx = ((int)x / 32);

  if (yy >= 0 && yy < height && xx >= 0 && xx <= width)
    c = ia_tiles[yy * width + xx];
  else
    c = 0;

  return c;
}

unsigned int
Level::get_tile_at(int x, int y) const
{
  if(x < 0 || x >= width || y < 0 || y >= height)
    return 0;
  
  return ia_tiles[y * width + x];
}

/* EOF */
