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

#ifndef SUPERTUX_HIGH_SCORES_H
#define SUPERTUX_HIGH_SCORES_H

#include <stdio.h>

extern int hs_score;
extern std::string hs_name; /* highscores global variables*/

void save_hs(int score);
void load_hs();

#endif
