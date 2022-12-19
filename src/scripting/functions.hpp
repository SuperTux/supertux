//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_FUNCTIONS_HPP
#define HEADER_SUPERTUX_SCRIPTING_FUNCTIONS_HPP

#ifndef SCRIPTING_API
#include <squirrel.h>
#include <string>

#define __suspend
#define __custom(x)
#endif

namespace scripting {

/** Display the value of the argument. This is useful for inspecting tables. */
SQInteger display(HSQUIRRELVM vm) __custom("t.");

/** Displays contents of the current stack */
void print_stacktrace(HSQUIRRELVM vm);

/** returns the currently running thread */
SQInteger get_current_thread(HSQUIRRELVM vm) __custom("t");

/** Should use christmas mode */
bool is_christmas();

/** Display a text file and scrolls it over the screen (on next screenswitch) */
void display_text_file(const std::string& filename);

/** Load and display a worldmap (on next screenswitch) */
void load_worldmap(const std::string& filename);

/** Switch to a different worldmap after unloading current one, after exit_screen() is called */
void set_next_worldmap(const std::string& dirname, const std::string& spawnpoint);

/** Load and display a level (on next screenswitch) */
void load_level(const std::string& filename);

/** Manages skippable cutscenes (cancels calls to wait()) */
void start_cutscene();
void end_cutscene();
bool check_cutscene();

/** Suspend the script execution for the specified number of seconds */
void wait(HSQUIRRELVM vm, float seconds) __suspend;

/** Suspend the script execution until the current screen has been changed */
void wait_for_screenswitch(HSQUIRRELVM vm) __suspend;

/** Exits the currently running screen (force exit from worldmap or scrolling text for example) */
void exit_screen();

/** Translate a text into the users language (by looking it up in the .po files) */
std::string translate(const std::string& text);
std::string _(const std::string& text);

std::string translate_plural(const std::string& text, const std::string&
    text_plural, int num);
std::string __(const std::string& text, const std::string& text_plural, int num);

/** Load a script file and executes it. This is typically used to import functions from external files. */
void import(HSQUIRRELVM v, const std::string& filename);

/** Save world state to scripting table */
void save_state();

/** Load world state from scripting table */
void load_state();

/** enable/disable drawing of collision rectangles */
void debug_collrects(bool enable);

/** enable/disable drawing of fps */
void debug_show_fps(bool enable);

/** enable/disable drawing of non-solid layers */
void debug_draw_solids_only(bool enable);

/** enable/disable drawing of editor images */
void debug_draw_editor_images(bool enable);

/** enable/disable worldmap ghost mode */
void debug_worldmap_ghost(bool enable);

/** Changes music to musicfile */
void play_music(const std::string& musicfile);

/** Stops the music */
void stop_music(float fadetime);

/** Fade in music */
void fade_in_music(const std::string& musicfile, float fadetime);

/** Resume music */
void resume_music(float fadetime);

/** Pause music **/
void pause_music(float fadetime);

/** Plays a soundfile */
void play_sound(const std::string& soundfile);

void stop_sound();

/**  Set the game_speed */
void set_game_speed(float speed);

/** speeds Tux up */
void grease();

/** makes Tux invincible for 10000 units of time */
void invincible();

/** makes Tux a ghost, i.e. lets him float around and through solid objects */
void ghost();

/** recall Tux's invincibility and ghost status */
void mortal();

/** reinitialise and respawn Tux at the beginning of the current level */
void restart();

/** print Tux's current coordinates in a level */
void whereami();

/** move Tux near the end of the level */
void gotoend();

/** move Tux to the X and Y blocks relative to his position */
void warp(float offset_x, float offset_y);

/** show the camera's coordinates */
void camera();

/** adjust gamma */
void set_gamma(float gamma);

/** exit the game */
void quit();

/** Returns a random integer */
int rand();

/** Record a demo to the given file. */
void record_demo(const std::string& filename);

/** Play back a demo from the given file. */
void play_demo(const std::string& filename);

} // namespace scripting

#endif

/* EOF */
