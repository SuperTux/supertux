#include <stdio.h>
#include <string>
#include <squirrel.h>
#include "textscroller.h"
#include "functions.h"
#include "script_interpreter.h"
#include "tinygettext/tinygettext.h"
#include "resources.h"
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

void display_text_file(const std::string& filename)
{
  std::string file 
    = ScriptInterpreter::current()->get_working_directory() + filename;
  ::display_text_file(file);
}

}

