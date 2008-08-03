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

#ifndef SUPERTUX_LEVEL_H
#define SUPERTUX_LEVEL_H

#include <string>
#include "texture.h"
#include "badguy.h"
#include "lispreader.h"
#ifndef NOSOUND
#include "musicref.h"
#endif

class Tile;

/** This type holds meta-information about a level-subset. 
    It could be extended to handle manipulation of subsets. */
class LevelSubset
  {
  public:
    LevelSubset();
    ~LevelSubset();

    static void create(const std::string& subset_name);
    void load(char *subset);
    void save();

    std::string name;
    std::string title;
    std::string description;
    Surface* image;
    int levels;
 
  private:
    void parse(lisp_object_t* cursor);
  };

#define LEVEL_NAME_MAX 20


enum TileMapType {
 TM_BG,
 TM_IA,
 TM_FG
 };

struct ResetPoint
{
  int x;
  int y;
};

class Level 
{
 public:
  Surface* img_bkgd;
#ifndef NOSOUND
  MusicRef level_song;
  MusicRef level_song_fast;
#endif

  std::string name;
  std::string author;
  std::string song_title;
  std::string bkgd_image;
  std::string particle_system;
  std::vector<unsigned int> bg_tiles[15]; /* Tiles in the background */
  std::vector<unsigned int> ia_tiles[15]; /* Tiles which can interact in the game (solids for example)*/
  std::vector<unsigned int> fg_tiles[15]; /* Tiles in the foreground */
  int time_left;
  Color bkgd_top;
  Color bkgd_bottom;
  int width;
  int bkgd_speed;
  int start_pos_x;
  int start_pos_y;
  float gravity;
  bool back_scrolling;
  float hor_autoscroll_speed;

  std::vector<BadGuyData> badguy_data;

  /** A collection of points to which Tux can be reset after a lost live */
  std::vector<ResetPoint> reset_points;
 public:
  Level();
  Level(const std::string& subset, int level);
  Level(const std::string& filename);
  ~Level();

  /** Will the Level structure with default values */
  void init_defaults();
  
  /** Cleanup the level struct from allocated tile data and such */
  void cleanup();

  /** Load data for this level: 
      Returns -1, if the loading of the level failed. */
  int  load(const std::string& subset, int level);

  /** Load data for this level: 
      Returns -1, if the loading of the level failed. */
  int  load(const std::string& filename);

  void load_gfx();
#ifndef NOSOUND
  void load_song();
  void free_song();
  MusicRef get_level_music();
  MusicRef get_level_music_fast();
#endif

  void save(const std::string& subset, int level);

  /** Edit a piece of the map! */
  void change(float x, float y, int tm, unsigned int c);

  /** Resize the level to a new width */
  void change_size (int new_width);

  /** Return the id of the tile at position x/y */
  unsigned int gettileid(float x, float y) const;
  /** returns the id of the tile at position x,y
   * (these are logical and not pixel coordinates)
   */
  unsigned int get_tile_at(int x, int y) const;

  void load_image(Surface** ptexture, std::string theme, const char * file, int use_alpha);
};

#endif /*SUPERTUX_LEVEL_H*/
