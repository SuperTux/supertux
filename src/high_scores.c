/*
 
  by Adam Czachorowski
  gislan@o2.pl
 
*/

/* Open the highscore file: */

#include <string.h>
#include <stdlib.h>

#include "globals.h"
#include "high_scores.h"

FILE * opendata(char * mode)
{
  char * filename;
  FILE * fi;


#ifdef LINUX

  /* Create the buffer for the filename: */

  filename = (char *) malloc(sizeof(char) * (strlen(st_dir) +
                             strlen("/highscore") + 1));

  strcpy(filename, st_dir);
  /* Open the high score file: */

  strcat(filename, "/highscore");
#else

  filename = "st_highscore.dat";
#endif


  /* Try opening the file: */

  fi = fopen(filename, mode);

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

int load_hs(void)
{
  FILE * fi;
  char temp[128];
  int score = 100;

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
                  score = atoi(temp + 10);

                  if (score == 0)
                    score = 100;
                }
            }
        }
      while (!feof(fi));

      fclose(fi);
    }
  return score;
}

void save_hs(int score)
{
  FILE * fi;


  /* Try to open file: */

  fi = opendata("w");
  if (fi != NULL)
    {
      fprintf(fi, "# Supertux highscore file\n\n");

      fprintf(fi, "highscore=%d\n", score);

      fprintf(fi, "# (File automatically created.)\n");

      fclose(fi);
    }
}
