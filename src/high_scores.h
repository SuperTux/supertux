/*

  by Adam Czachorowski
  gislan@o2.pl

*/

#ifndef SUPERTUX_HIGH_SCORES_H
#define SUPERTUX_HIGH_SCORES_H

#include <stdio.h>

extern int hs_score;
extern std::string hs_name; /* highscores global variables*/

void save_hs(int score);
void load_hs();

#endif
