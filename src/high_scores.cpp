//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Adam Czachorowski <gislan@o2.pl>
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

/* Open the highscore file: */

#include <string.h>
#include <stdlib.h>

#include "globals.h"
#include "high_scores.h"
#include "menu.h"
#include "screen.h"
#include "texture.h"
#include "setup.h"
#include "lispreader.h"

#ifdef WIN32
const char * highscore_filename = "/st_highscore.dat";
#else
const char * highscore_filename = "/highscore";
#endif

int hs_score;
std::string hs_name; /* highscores global variables*/

/* Load data from high score file: */

void load_hs(void)
{
  hs_score = 100;
  hs_name  = "Grandma";

  FILE * fi;
  lisp_object_t* root_obj = 0;
  fi = fopen(highscore_filename, "r");
  if (fi == NULL)
    {
      perror(highscore_filename);
      return;
    }

  lisp_stream_t stream;
  lisp_stream_init_file (&stream, fi);
  root_obj = lisp_read (&stream);

  if (root_obj->type == LISP_TYPE_EOF || root_obj->type == LISP_TYPE_PARSE_ERROR)
    {
      printf("HighScore: Parse Error in file %s", highscore_filename);
    }


  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-highscore") == 0)
    {
      LispReader reader(lisp_cdr(root_obj));
      reader.read_int("score",  &hs_score);
      reader.read_string("name", &hs_name);
    }
 
  fclose(fi);
  lisp_free(root_obj);
}

void save_hs(int score)
{
  char str[80];

  Surface* bkgd;
  SDL_Event event;

  bkgd = new Surface(datadir + "/images/highscore/highscore.png", IGNORE_ALPHA);

  hs_score = score;

  Menu::set_current(highscore_menu);

  if(!highscore_menu->item[0].input)
    highscore_menu->item[0].input = (char*) malloc(strlen(hs_name.c_str()) + 1);

  strcpy(highscore_menu->item[0].input,hs_name.c_str());

  /* ask for player's name */
  while(Menu::current())
    {
      bkgd->draw_bg();

      blue_text->drawf("Congratulations", 0, 130, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      blue_text->draw("Your score:", 150, 180, 1, NO_UPDATE);
      sprintf(str, "%d", hs_score);
      yellow_nums->draw(str, 350, 170, 1, NO_UPDATE);

      Menu::current()->draw();
      Menu::current()->action();

      flipscreen();

      while(SDL_PollEvent(&event))
        if(event.type == SDL_KEYDOWN)
          Menu::current()->event(event);

      switch (highscore_menu->check())
        {
        case 0:
          if(highscore_menu->item[0].input != NULL)
            hs_name = highscore_menu->item[0].input;
          break;
        }

      SDL_Delay(25);
    }


  /* Save to file: */

  FILE* fi;
  std::string filename;

  /* Save data file: */
  filename = highscore_filename;

  fcreatedir(filename.c_str());
  if(fwriteable(filename.c_str()))
    {
      fi = fopen(filename.c_str(), "w");
      if (fi == NULL)
        {
          perror(filename.c_str());
        }

      /* Write header: */
      fprintf(fi,";SuperTux HighScores\n");
      fprintf(fi,"(supertux-highscore\n");

      /* Save title info: */
      fprintf(fi,"  (name \"%s\")\n", hs_name.c_str());

      /* Save the description: */
      fprintf(fi,"  (score \"%i\")\n", hs_score);

      fprintf( fi,")");
      fclose(fi);
    }

/*
  fi = opendata(highscore_filename, "w");
  if (fi != NULL)
    {
      fprintf(fi, "# Supertux highscore file\n\n");

      fprintf(fi, "name=%s\n", hs_name);
      fprintf(fi, "highscore=%d\n", hs_score);

      fprintf(fi, "# (File automatically created.)\n");

      fclose(fi);
    }*/
}
