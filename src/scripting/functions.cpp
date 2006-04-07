#include <stdio.h>
#include <string>
#include <squirrel.h>
#include <sqstdio.h>
#include "textscroller.hpp"
#include "functions.hpp"
#include "script_interpreter.hpp"
#include "tinygettext/tinygettext.hpp"
#include "resources.hpp"
#include "gettext.hpp"
#include "msg.hpp"
#include "mainloop.hpp"

namespace Scripting
{

void wait(float seconds)
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
  main_loop->push_screen(new TextScroller(file));
}

void import(HSQUIRRELVM v, const std::string& filename)
{
  std::string file 
    = ScriptInterpreter::current()->get_working_directory() + filename;
  if(sqstd_loadfile(v, file.c_str(), true) < 0) {
    msg_warning("couldn't load script '" << filename << "' ("
      << file << ")");
    return;
  }

  sq_push(v, -2);
  if(sq_call(v, 1, false) < 0) {
    msg_warning("Couldn't execute script '" << filename << "' ("
      << file << ")");
    return;
  }
}

void add_key(int new_key)
{
  player_status->set_keys(new_key);
}

}

