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

#ifdef DOXYGEN_SCRIPTING
/**
 * @summary This module contains global methods.
 */
class Globals
{
public:
#endif

/**
 * Displays the value of an argument. This is useful for inspecting tables.
 * @param ANY $object
 */
SQInteger display(HSQUIRRELVM vm) __custom("t.");

/**
 * Displays the contents of the current stack.
 */
void print_stacktrace(HSQUIRRELVM vm);

/**
 * Returns the currently running thread.
 */
SQInteger get_current_thread(HSQUIRRELVM vm) __custom("t");

/**
 * Returns whether the game is in christmas mode.
 */
bool is_christmas();

/**
 * Displays a text file and scrolls it over the screen (on next screenswitch).
 * @param string $filename
 */
void display_text_file(const std::string& filename);

/**
 * Loads and displays a worldmap (on next screenswitch).
 * @param string $filename
 */
void load_worldmap(const std::string& filename);

/**
 * Switches to a different worldmap after unloading the current one, after ""exit_screen()"" is called.
 * @param string $dirname
 * @param string $spawnpoint
 */
void set_next_worldmap(const std::string& dirname, const std::string& spawnpoint);

/**
 * Loads and displays a level (on next screenswitch).
 * @param string $filename
 */
void load_level(const std::string& filename);

/**
 * Starts a skippable cutscene.
 */
void start_cutscene();
/**
 * Ends a skippable cutscene.
 */
void end_cutscene();
/**
 * Checks if a skippable cutscene is currently running.
 */
bool check_cutscene();

/**
 * Suspends the script execution for a specified number of seconds.
 * @param float $seconds
 */
void wait(HSQUIRRELVM vm, float seconds) __suspend;

/**
 * Suspends the script execution until the current screen has been changed.
 */
void wait_for_screenswitch(HSQUIRRELVM vm) __suspend;

/**
 * Exits the currently running screen (for example, force exits from worldmap or scrolling text).
 */
void exit_screen();

/**
 * Translates a text into the user's language (by looking in the "".po"" files).
 * @param string $text
 */
std::string translate(const std::string& text);
/**
 * Same function as ""translate()"".
 * @param string $text
 */
std::string _(const std::string& text);

/**
 * Translates a text into the user's language (by looking in the "".po"" files).
   Returns ""text"" or ""text_plural"", depending on ""num"" and the locale.
 * @param string $text
 * @param string $text_plural
 * @param int $num
 */
std::string translate_plural(const std::string& text, const std::string&
    text_plural, int num);
/**
 * Same function as ""translate_plural()"".
 * @param string $text
 * @param string $text_plural
 * @param int $num
 */
std::string __(const std::string& text, const std::string& text_plural, int num);

/**
 * Loads a script file and executes it. This is typically used to import functions from external files.
 * @param string $filename
 */
void import(HSQUIRRELVM v, const std::string& filename);

/**
 * Saves world state to scripting table.
 */
void save_state();

/**
 * Loads world state from scripting table.
 */
void load_state();

/**
 * Enables/disables drawing of collision rectangles.
 * @param bool $enable
 */
void debug_collrects(bool enable);

/**
 * Enables/disables drawing of FPS.
 * @param bool $enable
 */
void debug_show_fps(bool enable);

/**
 * Enables/disables drawing of non-solid layers.
 * @param bool $enable
 */
void debug_draw_solids_only(bool enable);

/**
 * Enables/disables drawing of editor images.
 * @param bool $enable
 */
void debug_draw_editor_images(bool enable);

/**
 * Enables/disables worldmap ghost mode.
 * @param bool $enable
 */
void debug_worldmap_ghost(bool enable);

/**
 * Changes the music to ""musicfile"".
 * @param string $musicfile
 */
void play_music(const std::string& musicfile);

/**
 * Fades in the music from ""musicfile"" for ""fadetime"" seconds.
 * @param string $musicfile
 * @param float $fadetime
 */
void fade_in_music(const std::string& musicfile, float fadetime);

/**
 * Fades out the music for ""fadetime"" seconds.
 * @param float $fadetime Set to "0" for no fade-out.
 */
void stop_music(float fadetime);

/**
 * Resumes and fades in the music for ""fadetime"" seconds.
 * @param float $fadetime Set to "0" for no fade-in.
 */
void resume_music(float fadetime);

/**
 * Pauses the music with a fade-out for ""fadetime"" seconds.
 * @param float $fadetime Set to "0" for no fade-out.
 */
void pause_music(float fadetime);

/**
 * Plays ""soundfile"" as a sound.
 * @param string $soundfile
 */
void play_sound(const std::string& soundfile);

/**
 * Sets the game speed to ""speed"".
 * @param float $speed
 */
void set_game_speed(float speed);

/**
 * Speeds Tux up.
 */
void grease();

/**
 * Makes Tux invincible for 10000 units of time.
 */
void invincible();

/**
 * Makes Tux a ghost, i.e. lets him float around and through solid objects.
 */
void ghost();

/**
 * Recalls Tux's invincibility and ghost status.
 */
void mortal();

/**
 * Re-initializes and respawns Tux at the beginning of the current level.
 */
void restart();

/**
 * Prints Tux's current coordinates in the current level.
 */
void whereami();

/**
 * Moves Tux near the end of the current level.
 */
void gotoend();

/**
 * Moves Tux to the X and Y blocks, relative to his position.
 * @param float $offset_x
 * @param float $offset_y
 */
void warp(float offset_x, float offset_y);

/**
 * Shows the camera's coordinates.
 */
void camera();

/**
 * Adjusts the gamma.
 * @param float $gamma
 */
void set_gamma(float gamma);

/**
 * Returns a random integer.
 */
int rand();

/**
 * Records a demo to the given file.
 * @param string $filename
 */
void record_demo(const std::string& filename);

/**
 * Plays back a demo from the given file.
 * @param string $filename
 */
void play_demo(const std::string& filename);

/**
 * Sets the frame, displayed on the title screen.
 * @param string $image
 */
void set_title_frame(const std::string& image);

#ifdef DOXYGEN_SCRIPTING
}
#endif

} // namespace scripting

#endif

/* EOF */
