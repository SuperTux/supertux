#include <stdio.h>
#include <string>
#include <squirrel.h>
#include "functions.h"
#include "script_interpreter.h"
#include "tinygettext/tinygettext.h"
#include "gettext.h"

namespace Scripting
{

void set_wakeup_time(float seconds)
{
  ScriptInterpreter::current()->set_wakeup_time(seconds);
}

std::string translate(const std::string& text)
{
  return dictionary_manager.get_dictionary().translate(text);
}

}

