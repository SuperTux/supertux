//  $Id$
// 
//  SuperTux
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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
#include <algorithm>
#include <iostream>
#include "world.h"
#include "tile.h"
#include "gameloop.h"
#include "gameobjs.h"
#include "sprite_manager.h"
#include "resources.h"
#include "level.h"

void
BouncyDistro::init(float x, float y)
{
  base.x = x;
  base.y = y;
  base.ym = -2;
}

void
BouncyDistro::action(double frame_ratio)
{
  base.y = base.y + base.ym * frame_ratio;

  base.ym += 0.1 * frame_ratio;

  if (base.ym >= 0)
    {
      std::vector<BouncyDistro*>::iterator i
        = std::find(World::current()->bouncy_distros.begin(), 
                    World::current()->bouncy_distros.end(), 
                    this);
      if (i != World::current()->bouncy_distros.end())
        World::current()->bouncy_distros.erase(i);
    }
}

void
BouncyDistro::draw()
{
  img_distro[0]->draw(base.x - scroll_x,
                      base.y - scroll_y);
}


void
BrokenBrick::init(Tile* tile_, float x, float y, float xm, float ym)
{
  tile    = tile_;
  base.x  = x;
  base.y  = y;
  base.xm = xm;
  base.ym = ym;

  timer.init(true);
  timer.start(200);
}

void
BrokenBrick::action(double frame_ratio)
{
  base.x = base.x + base.xm * frame_ratio;
  base.y = base.y + base.ym * frame_ratio;

  if (!timer.check())
    {
      std::vector<BrokenBrick*>::iterator i
        = std::find(World::current()->broken_bricks.begin(), 
                    World::current()->broken_bricks.end(), 
                    this);
      if (i != World::current()->broken_bricks.end())
        World::current()->broken_bricks.erase(i);
    }
}

void
BrokenBrick::draw()
{
  SDL_Rect src, dest;
  src.x = rand() % 16;
  src.y = rand() % 16;
  src.w = 16;
  src.h = 16;

  dest.x = (int)(base.x - scroll_x);
  dest.y = (int)(base.y  - scroll_y);
  dest.w = 16;
  dest.h = 16;
  
  if (tile->images.size() > 0)
    tile->images[0]->draw_part(src.x,src.y,dest.x,dest.y,dest.w,dest.h);
}

void
BouncyBrick::init(float x, float y)
{
  base.x   = x;
  base.y   = y;
  offset   = 0;
  offset_m = -BOUNCY_BRICK_SPEED;
  shape    = World::current()->get_level()->gettileid(x, y);
}

void
BouncyBrick::action(double frame_ratio)
{
  offset = (offset + offset_m * frame_ratio);

  /* Go back down? */
  if (offset < -BOUNCY_BRICK_MAX_OFFSET)
    offset_m = BOUNCY_BRICK_SPEED;


  /* Stop bouncing? */
  if (offset >= 0)
    {
      std::vector<BouncyBrick*>::iterator i
        = std::find(World::current()->bouncy_bricks.begin(), 
                    World::current()->bouncy_bricks.end(), 
                    this);
      if (i != World::current()->bouncy_bricks.end())
        World::current()->bouncy_bricks.erase(i);
    }
}

void
BouncyBrick::draw()
{
  SDL_Rect dest;
  
  if (base.x >= scroll_x - 32 &&
      base.x <= scroll_x + screen->w)
    {
      dest.x = (int)(base.x - scroll_x);
      dest.y = (int)(base.y - scroll_y);
      dest.w = 32;
      dest.h = 32;

      Level* plevel = World::current()->get_level();

      // FIXME: overdrawing hack to clean the tile from the screen to
      // paint it later at on offseted position
      if(plevel->bkgd_image[0] == '\0')
        {
          fillrect(base.x - scroll_x, base.y - scroll_y,
                   32,32, 
                   plevel->bkgd_top.red, plevel->bkgd_top.green, plevel->bkgd_top.blue, 0);
// FIXME: doesn't respect the gradient, futhermore is this necessary at all??
        }
      else
        {
          int s = ((int)scroll_x / 2)%640;
          plevel->img_bkgd->draw_part(dest.x + s, dest.y, 
                                      dest.x, dest.y,dest.w,dest.h);
        }

      Tile::draw(base.x - scroll_x,
                 base.y - scroll_y + offset,
                 shape);
    }
}

void
FloatingScore::init(float x, float y, int s)
{
  base.x = x;
  base.y = y - 16;
  timer.init(true);
  timer.start(1000);
  value = s;
}

void
FloatingScore::action(double frame_ratio)
{
  base.y = base.y - 2 * frame_ratio;

  if(!timer.check())
    {
      std::vector<FloatingScore*>::iterator i
        = std::find(World::current()->floating_scores.begin(), 
                    World::current()->floating_scores.end(), 
                    this);
      if (i != World::current()->floating_scores.end())
        World::current()->floating_scores.erase(i);
    }
}

void
FloatingScore::draw()
{
  char str[10];
  sprintf(str, "%d", value);
  gold_text->draw(str, (int)base.x + 16 - strlen(str) * 8, (int)base.y, 1);
}

/* Trampoline */

#define TRAMPOLINE_FRAMES 4
Sprite *img_trampoline[TRAMPOLINE_FRAMES];

void load_object_gfx()
{
  char sprite_name[16];

  for (int i = 0; i < TRAMPOLINE_FRAMES; i++)
  {
    sprintf(sprite_name, "trampoline-%i", i+1);
    img_trampoline[i] = sprite_manager->load(sprite_name);
  }
}

void
Trampoline::init(float x, float y)
{
  base.x = x;
  base.y = y;
  base.width = 32;
  base.height = 32;

  frame = 0;
  mode = M_NORMAL;
  physic.reset();
}

void
Trampoline::draw()
{
  img_trampoline[frame]->draw((int)base.x, (int)base.y);

  frame = 0;

  if (debug_mode)
    fillrect(base.x - scroll_x, base.y - scroll_y, base.width, base.height, 75, 75, 0, 150);
}

void
Trampoline::action(double frame_ratio)
{
  // TODO: Remove if we're too far off the screen

  // Falling
  if (mode != M_HELD)
  {
    if (issolid(base.x + base.width/2, base.y + base.height))
    {
      base.y = int((base.y + base.height)/32) * 32 - base.height;

      physic.enable_gravity(false);
      physic.set_velocity_y(0.0f);

      physic.set_velocity_x(0);
    }
    else
    {
      physic.enable_gravity(true);
    }
  }
  else // Player is carrying us around
  {
    /* FIXME: The trampoline object shouldn't know about pplayer objects. */
    /* If we're holding the iceblock */
    Player& tux = *World::current()->get_tux();
    Direction dir = tux.dir;

    if(dir == RIGHT)
    {
      base.x = tux.base.x + 16;
      base.y = tux.base.y + tux.base.height/1.5 - base.height;
    }
    else /* facing left */
    {
      base.x = tux.base.x - 16;
      base.y = tux.base.y + tux.base.height/1.5 - base.height;
    }

    if(collision_object_map(base))
    {
      base.x = tux.base.x;
      base.y = tux.base.y + tux.base.height/1.5 - base.height;
    }
  }

  physic.apply(frame_ratio, base.x, base.y);
  collision_swept_object_map(&old_base, &base);
}

void
Trampoline::collision(void *p_c_object, int c_object, CollisionType type)
{
  Player* pplayer_c = NULL;
  switch (c_object)
  {
    case CO_PLAYER:
      pplayer_c = (Player*) p_c_object;

      if (type == COLLISION_NORMAL)
      {
        // Pick up if HELD (done in Player)
      }

      else if (type == COLLISION_SQUISH)
      {
        int squish_amount = (32 - (int)pplayer_c->base.y % 32);

        if (squish_amount < 24)
          frame = 3;
        else if (squish_amount < 28)
          frame = 2;
        else if (squish_amount < 30)
          frame = 1;
        else
          frame = 0;

        if (squish_amount < 20)
          pplayer_c->physic.set_velocity_y(power);
        else if (pplayer_c->physic.get_velocity_y() < 0)
          pplayer_c->physic.set_velocity_y(-squish_amount/32);
      }

      break;

    default:
      break;
    
  }
}


/* Object Manager */
//---------------------------------------------------------------------------

ObjectManager::ObjectManager()
{
  std::string filename = datadir + "/images/tilesets/supertux.stbg";
  load_badguys(filename);
}

ObjectManager::~ObjectManager()
{
  for(std::vector<BadGuy*>::iterator i = badguys.begin(); i != badguys.end(); ++i) {
    delete *i;                                                                  
  }
}

void ObjectManager::load_badguys(std::string filename)
{
/*
  lisp_object_t* root_obj = lisp_read_from_file(filename);

  if (!root_obj)
    st_abort("Couldn't load file", filename);

  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-badguys") == 0)
    {
      lisp_object_t* cur = lisp_cdr(root_obj);

      while(!lisp_nil_p(cur))
        {
          lisp_object_t* element = lisp_car(cur);

          if (strcmp(lisp_symbol(lisp_car(element)), "badguy") == 0)
            {
	      
	     
              Tile* tile = new Tile;
              tile->id      = -1;
              tile->solid   = false;
              tile->brick   = false;
              tile->ice     = false;
              tile->water   = false;
              tile->fullbox = false;
              tile->distro  = false;
              tile->goal    = false;
              tile->data    = 0;
              tile->next_tile  = 0;
              tile->anim_speed = 25;

              LispReader reader(lisp_cdr(element));
              assert(reader.read_int("id",  &tile->id));
              reader.read_bool("solid",     &tile->solid);
              reader.read_bool("brick",     &tile->brick);
              reader.read_bool("ice",       &tile->ice);
              reader.read_bool("water",     &tile->water);
              reader.read_bool("fullbox",   &tile->fullbox);
              reader.read_bool("distro",    &tile->distro);
              reader.read_bool("goal",      &tile->goal);
              reader.read_int("data",       &tile->data);
              reader.read_int("anim-speed", &tile->anim_speed);
              reader.read_int("next-tile",  &tile->next_tile);
              reader.read_string_vector("images",  &tile->filenames);
	      reader.read_string_vector("editor-images", &tile->editor_filenames);

              for(std::vector<std::string>::iterator it = tile->
                  filenames.begin();
                  it != tile->filenames.end();
                  ++it)
                {
                  Surface* cur_image;
                  tile->images.push_back(cur_image);
                  tile->images[tile->images.size()-1] = new Surface(
                               datadir +  "/images/tilesets/" + (*it),
                               USE_ALPHA);
                }
              for(std::vector<std::string>::iterator it = tile->editor_filenames.begin();
                  it != tile->editor_filenames.end();
                  ++it)
                {
                  Surface* cur_image;
                  tile->editor_images.push_back(cur_image);
                  tile->editor_images[tile->editor_images.size()-1] = new Surface(
                               datadir + "/images/tilesets/" + (*it),
                               USE_ALPHA);
                }
		
              if (tile->id + tileset_id >= int(tiles.size())
                 )
                tiles.resize(tile->id + tileset_id+1);

              tiles[tile->id + tileset_id] = tile;
            }
          else if (strcmp(lisp_symbol(lisp_car(element)), "tileset") == 0)
            {
              LispReader reader(lisp_cdr(element));
              std::string filename;
              reader.read_string("file",  &filename);
              filename = datadir + "/images/tilesets/" + filename;
              load_tileset(filename);
            }
          else if (strcmp(lisp_symbol(lisp_car(element)), "tilegroup") == 0)
            {
              TileGroup new_;
              LispReader reader(lisp_cdr(element));
              reader.read_string("name",  &new_.name);
              reader.read_int_vector("tiles", &new_.tiles);	      
              if(!tilegroups_)
                tilegroups_ = new std::set<TileGroup>;
              tilegroups_->insert(new_).first;
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

  lisp_free(root_obj);
*/
}

/* EOF */

