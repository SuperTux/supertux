/*

  by Adam Czachorowski
  gislan@o2.pl

*/

# include <stdio.h>

int hs_score;
char hs_name[62]; /* highscores global variables*/

void save_hs(int score);
void load_hs();
FILE * opendata(char * mode);
