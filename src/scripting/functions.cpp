#include <stdio.h>
#include <string>
#include <squirrel.h>
#include <sqstdio.h>
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

void import(HSQUIRRELVM v, const std::string& filename)
{
  std::string file 
    = ScriptInterpreter::current()->get_working_directory() + filename;
  if(sqstd_loadfile(v, file.c_str(), true) < 0) {
    std::cerr << "Warning couldn't load script '" << filename << "' ("
      << file << ").\n";
    return;
  }

  sq_push(v, -2);
  if(sq_call(v, 1, false) < 0) {
    std::cerr << "Couldn't execute script '" << filename << "' ("
      << file << ").\n";
    return;
  }
}

}

