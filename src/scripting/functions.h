#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

namespace Scripting
{

/** Suspends the script execution for the specified number of seconds */
void set_wakeup_time(float seconds);
/** translates a give text into the users language (by looking it up in the .po
 * files)
 */
std::string translate(const std::string& text);

}

#endif

