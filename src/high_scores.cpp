/*
 
  by Adam Czachorowski
  gislan@o2.pl
 
*/

/* Open the highscore file: */

#include <string.h>
#include <stdlib.h>

#include "globals.h"
#include "high_scores.h"
#include "menu.h"
#include "screen.h"
#include "texture.h"
#include "setup.h"

#ifdef WIN32
const char * highscore_filename = "/st_highscore.dat";
#else
const char * highscore_filename = "/highscore";
#endif

int hs_score;
char hs_name[62]; /* highscores global variables*/

/* Load data from high score file: */

void load_hs(void)
{
  FILE * fi;
  char temp[128];
  int strl;
  unsigned int i, c;

  hs_score = 100;
  strcpy(hs_name, "Grandma\0");
  c = 0;

  /* Try to open file: */

  fi = opendata(highscore_filename, "r");
  if (fi != NULL)
    {
      do
        {
          fgets(temp, sizeof(temp), fi);

          if (!feof(fi))
            {
              temp[strlen(temp) - 1] = '\0';


              /* Parse each line: */

              if (strstr(temp, "highscore=") == temp)
                {
                  hs_score = atoi(temp + 10);

                  if (hs_score == 0)
                    hs_score = 100;
                }
              if (strstr(temp, "name=") == temp)
                {
                  fprintf(stderr, "name found\n");
                  strl = strlen("name=");
                  for(c = strl, i = 0; c < strlen(temp); ++c, ++i)
                    hs_name[i] = temp[c];
                  hs_name[i]= '\0';
                }
            }
        }
      while (!feof(fi));

      fclose(fi);
    }
}

void save_hs(int score)
{
  char str[80];

  Surface* bkgd;
  SDL_Event event;
  FILE * fi;

  bkgd = new Surface(datadir + "/images/highscore/highscore.png", IGNORE_ALPHA);

  hs_score = score;

  menu_reset();
  Menu::set_current(highscore_menu);

  if(!highscore_menu->item[0].input)
    highscore_menu->item[0].input = (char*) malloc(strlen(hs_name) + 1);

  strcpy(highscore_menu->item[0].input,hs_name);

  /* ask for player's name */
  show_menu = 1;
  while(show_menu)
    {
      bkgd->draw_bg();

      blue_text->drawf("Congratulations", 0, 130, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
      blue_text->draw("Your score:", 150, 180, 1, NO_UPDATE);
      sprintf(str, "%d", hs_score);
      yellow_nums->draw(str, 350, 170, 1, NO_UPDATE);

      menu_process_current();
      flipscreen();

      while(SDL_PollEvent(&event))
        if(event.type == SDL_KEYDOWN)
          current_menu->event(event);

      switch (highscore_menu->check())
        {
        case 0:
          if(highscore_menu->item[0].input != NULL)
            strcpy(hs_name, highscore_menu->item[0].input);
          break;
        }

      SDL_Delay(25);
    }


  /* Try to open file: */

  fi = opendata(highscore_filename, "w");
  if (fi != NULL)
    {
      fprintf(fi, "# Supertux highscore file\n\n");

      fprintf(fi, "name=%s\n", hs_name);
      fprintf(fi, "highscore=%d\n", hs_score);

      fprintf(fi, "# (File automatically created.)\n");

      fclose(fi);
    }
}
