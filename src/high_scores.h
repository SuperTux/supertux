/*

  by Adam Czachorowski
  gislan@o2.pl

*/

#include <stdio.h>

extern int hs_score;
extern char hs_name[62]; /* highscores global variables*/

void save_hs(int score);
void load_hs();
FILE * opendata(char * mode);
