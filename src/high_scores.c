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

FILE * opendata(char * mode)
{
  char * filename;
  FILE * fi;


  filename = (char *) malloc(sizeof(char) * (strlen(st_dir) +
                             strlen("/st_highscore.dat") + 1));

  strcpy(filename, st_dir);
  /* Open the high score file: */

#ifdef LINUX

  strcat(filename, "/highscore");
#else
#ifdef WIN32

  strcat(filename, "/st_highscore.dat");
#endif
#endif


  /* Try opening the file: */

  fi = fopen(filename, mode);
  free( filename );

  if (fi == NULL)
    {
      fprintf(stderr, "Warning: I could not open the high score file ");

      if (strcmp(mode, "r") == 0)
        fprintf(stderr, "for read!!!\n");
      else if (strcmp(mode, "w") == 0)
        fprintf(stderr, "for write!!!\n");

    }

  return(fi);
}

/* Load data from high score file: */

void load_hs(void)
{
  FILE * fi;
  char temp[128];
  hs_score = 100;
  int c, strl;
  strcpy(hs_name, "Grandma");
  c = 0;
  
  /* Try to open file: */

  fi = opendata("r");
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
		  hs_name[strl-1]='\0';
                  for(c = strl; c < strlen(temp); c++)
                    hs_name[c-strl] = temp[c];
                }
            }
        }
      while (!feof(fi));

      fclose(fi);
    }
}

void save_hs(int score)
{
  texture_type bkgd;
  texture_load(&bkgd, DATA_PREFIX "/images/highscore/highscore.png", IGNORE_ALPHA);

  hs_score = score;

  /* ask for player's name */
  menumenu = MENU_HIGHSCORE;
  show_menu = 1;
  SDL_Event event;
  while(show_menu)
    {
      texture_draw_bg(&bkgd, NO_UPDATE);
      drawmenu();
      flipscreen();

      while(SDL_PollEvent(&event))
        if(event.type == SDL_KEYDOWN)
          menu_event(&event.key.keysym);
    }


  FILE * fi;

  /* Try to open file: */

  fi = opendata("w");
  if (fi != NULL)
    {
      fprintf(fi, "# Supertux highscore file\n\n");

      fprintf(fi, "name=%s\n", hs_name);
      fprintf(fi, "highscore=%d\n", hs_score);

      fprintf(fi, "# (File automatically created.)\n");

      fclose(fi);
    }
}
