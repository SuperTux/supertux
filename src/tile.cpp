//
// C++ Implementation: tile
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "tile.h"
#include "assert.h"

TileManager* TileManager::instance_  = 0;

TileManager::TileManager()
{
  std::string filename = datadir +  "images/tilesets/supertux.stgt"; 
  load_tileset(filename);
}

void TileManager::load_tileset(std::string filename)
{
  lisp_object_t* root_obj = lisp_read_from_file(filename);
  
  if (!root_obj)
    st_abort("Couldn't load file", filename);

  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-tiles") == 0)
    {
      lisp_object_t* cur = lisp_cdr(root_obj);
      int tileset_id = 0;

      while(!lisp_nil_p(cur))
        {
          lisp_object_t* element = lisp_car(cur);

          if (strcmp(lisp_symbol(lisp_car(element)), "tile") == 0)
            {
              Tile* tile = new Tile;
              tile->id      = -1;
              tile->solid   = false;
	      tile->brick   = false;
	      tile->ice     = false;	  
	      tile->fullbox = false;	      
              tile->distro  = false;
              tile->data    = 0;
              tile->next_tile  = 0;
              tile->anim_speed = 25;
  
              LispReader reader(lisp_cdr(element));
              assert(reader.read_int("id",  &tile->id));
              reader.read_bool("solid",     &tile->solid);
              reader.read_bool("brick",     &tile->brick);
              reader.read_bool("ice",       &tile->ice);	   
              reader.read_bool("fullbox",   &tile->fullbox);
              reader.read_bool("distro",    &tile->distro);
              reader.read_int("data",       &tile->data);
              reader.read_int("anim-speed", &tile->anim_speed);
              reader.read_int("next-tile",  &tile->next_tile);
              reader.read_string_vector("images",  &tile->filenames);

	      for(std::vector<std::string>::iterator it = tile->filenames.begin();
                  it != tile->filenames.end();
                  ++it)
                {
                  texture_type cur_image;
                  tile->images.push_back(cur_image);
                  texture_load(&tile->images[tile->images.size()-1], 
                               datadir +  "images/tilesets/" + (*it), 
                               USE_ALPHA);
                }

              if (tile->id + tileset_id >= int(tiles.size()))
                tiles.resize(tile->id + tileset_id+1);

              tiles[tile->id + tileset_id] = tile;
            }
	  else if (strcmp(lisp_symbol(lisp_car(element)), "tileset") == 0)
            {
	      LispReader reader(lisp_cdr(element));
	      std::string filename;
              reader.read_string("file",  &filename);
	      filename = datadir + "images/tilesets/" + filename; 
	      load_tileset(filename);
	    }
	  else if (strcmp(lisp_symbol(lisp_car(element)), "properties") == 0)
            {
	      LispReader reader(lisp_cdr(element));
              reader.read_int("id",  &tileset_id);
	      tileset_id *= 1000;
	    }
          else
            {
              puts("Unhandled symbol");
            }

          cur = lisp_cdr(cur);
        }
    }
  else
    {
      assert(0);
    }
}

// EOF //

