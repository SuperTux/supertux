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
#include "badguy.h"
#include "lispreader.h"

class Tile;

/** This type holds meta-information about a level-subset. 
    It could be extended to handle manipulation of subsets. */
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


enum TileMapType {
 TM_BG,
 TM_IA,
 TM_FG
 };

extern texture_type img_bkgd;

/*
extern texture_type img_bkgd_tile[2][4];
extern texture_type img_solid[4];
extern texture_type img_brick[2];
*/
class Level 
{
 public:
  std::string name;
  std::string theme;
  std::string song_title;
  std::string bkgd_image;
  std::string particle_system;
  unsigned int* bg_tiles[15]; /* Tiles in the background */
  unsigned int* ia_tiles[15]; /* Tiles which can interact in the game (solids for example)*/
  unsigned int* fg_tiles[15]; /* Tiles in the foreground */
  int time_left;
  int bkgd_red;
  int bkgd_green;
  int bkgd_blue;
  int width;
  float gravity;

  std::vector<BadGuyData> badguy_data;
 public:
  /** Will the Level structure with default values */
  void init_defaults();
  
  /** Cleanup the level struct from allocated tile data and such */
  void cleanup();

  /** Load data for this level: 
      Returns -1, if the loading of the level failed. */
  int  load(const char* subset, int level);

  /** Load data for this level: 
      Returns -1, if the loading of the level failed. */
  int  load(const std::string& filename);

  void load_gfx();
  void load_song();

  void save(const char* subset, int level);

  /** Edit a piece of the map! */
  void change(float x, float y, int tm, unsigned int c);

  /** Resize the level to a new width */
  void change_size (int new_width);

  /** Return the id of the tile at position x/y */
  unsigned int gettileid(float x, float y);
};

void level_load_image(texture_type* ptexture, std::string theme, const char * file, int use_alpha);
void level_free_song();
void level_free_gfx();

#endif /*SUPERTUX_LEVEL_H*/
