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

class Level 
{
 public:
  Surface* img_bkgd;

  std::string name;
  std::string author;
  std::string theme;
  std::string song_title;
  std::string bkgd_image;
  std::string particle_system;
  unsigned int* bg_tiles[15]; /* Tiles in the background */
  unsigned int* ia_tiles[15]; /* Tiles which can interact in the game (solids for example)*/
  unsigned int* fg_tiles[15]; /* Tiles in the foreground */
  int time_left;
  Color bkgd_top;
  Color bkgd_bottom;
  int width;
  int  endpos;
  float gravity;

  std::vector<BadGuyData> badguy_data;
 public:
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
  
  void load_song();
  void free_song();

  void save(const char* subset, int level);

  /** Edit a piece of the map! */
  void change(float x, float y, int tm, unsigned int c);

  /** Resize the level to a new width */
  void change_size (int new_width);

  /** Return the id of the tile at position x/y */
  unsigned int gettileid(float x, float y);

  void free_gfx();

  void load_image(Surface** ptexture, std::string theme, const char * file, int use_alpha);
};

#endif /*SUPERTUX_LEVEL_H*/
