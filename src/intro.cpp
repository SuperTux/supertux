//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>

#include <sys/types.h>
#include <ctype.h>

#include "defines.h"
#include "globals.h"
#include "intro.h"
#include "screen.h"
#include "text.h"
#include "texture.h"
#include "timer.h"

char * intro_text[] = {
                        "Tux and Gown were having a nice picnic..",
                        "when suddenly...",
                        "Gown is beamed away!!!",
                        "This looks like a job for ---"
                      };


void draw_intro()
{
  bool done = false;

  while (!done)
    {
      SDL_Event event;
      drawgradient(Color(155, 200, 255), Color(255, 255, 255));

      gold_text->draw_align("Gwen gets captured!", screen->w/2, 15, A_HMIDDLE, A_TOP);

      white_text->draw
        ("\n"                                   //
         "Tux and Gwen were out having a nice\n"
         "picnic on the ice fields of Antarctica.\n"
         "Suddenly, a creature jumped from\n"
         "behind an ice bush, there was a flash,\n"
         "and Tux fell asleep!\n"
         "\n"                                   //
         "When Tux wakes up, he finds that Gwen\n"
         "is missing. Where she lay before now\n"
         "lies a letter. \"Tux, my arch enemy!\"\n"
         "says the letter. \"I have captured\n"
         "your beautiful Gwen and have taken her\n"
         "to my fortress. The path to my fortress\n"
         "is littered with my minions. Give up on the\n"
         "thought of trying to reclaim her, you\n"
         "haven't a chance! -Nolok\"\n"
         "\n"
         "Tux looks and see Nolok's fortress in\n"
         "the distance. Determined to save his\n"
         "beloved Gwen, he begins his journey.",
         -5, 30);

      white_small_text->draw_align("Press any button to continue!", 
                                   screen->w/2, screen->h - 20, A_HMIDDLE, A_TOP);

      flipscreen();
      SDL_Delay(100);

      done = wait_for_event(event,0);
    }
}

