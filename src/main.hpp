//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
#ifndef __MAIN_H__
#define __MAIN_H__

void init_video();
void wait_for_event(float min_delay, float max_delay);

/** The width of the display (this is a logical value, not the
    physical value, since aspect_ration and projection_area might
    shrink or scale things) */
extern int SCREEN_WIDTH;

/** The width of the display (this is a logical value, not the
    physical value, since aspect_ration and projection_area might
    shrink or scale things) */
extern int SCREEN_HEIGHT;

// global variables
class  JoystickKeyboardController;
extern JoystickKeyboardController* main_controller;

#endif
