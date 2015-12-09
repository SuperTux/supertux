//  SuperTux
//  Copyright (C) 2009 Mathnerd314
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SUPERTUX_SUPERTUX_CONSTANTS_HPP
#define HEADER_SUPERTUX_SUPERTUX_CONSTANTS_HPP

//Useful constants

// a small value... be careful as CD is very sensitive to it
static const float DELTA = .002f;

// the engine will be run with a logical framerate of 64fps.
// We chose 64fps here because it is a power of 2, so 1/64 gives an "even"
// binary fraction...
static const float LOGICAL_FPS = 64.0;

// SHIFT_DELTA is used for sliding over 1-tile gaps and collision detection
static const float SHIFT_DELTA = 7.0f;

#endif

/* EOF */
