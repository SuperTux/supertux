#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#ifndef SCRIPTING_API
#define __suspend
#include "player_status.hpp"
#endif

namespace Scripting
{

//TODO: Get this from PlayerStatus (update MiniSwig!)
static const int KEY_BRASS  = 0x001;
static const int KEY_IRON   = 0x002;
static const int KEY_BRONZE = 0x004;
static const int KEY_SILVER = 0x008;
static const int KEY_GOLD   = 0x010;

/** displays a text file and scrolls it over the screen */
void display_text_file(const std::string& filename);

/**
 * Suspends the script execution for the specified number of seconds
 */
void wait(float seconds) __suspend;

/** translates a give text into the users language (by looking it up in the .po
 * files)
 */
std::string translate(const std::string& text);

/** load a script file and executes it 
 * This is typically used to import functions from external files.
 */
void import(HSQUIRRELVM v, const std::string& filename);

/** add a key to the inventory
 */
void add_key(int new_key);

}

#endif

