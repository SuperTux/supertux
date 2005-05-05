#ifndef __SCRIPT_INTERPRETER_H__
#define __SCRIPT_INTERPRETER_H__

#include <squirrel.h>
#include <iostream>
#include "timer.h"

class ScriptInterpreter
{
public:
  ScriptInterpreter();
  ~ScriptInterpreter();

  void load_script(std::istream& in, const std::string& sourcename = "");
  void run_script();
  
  void expose_object(void* object, const std::string& name,
                     const std::string& type);

  void suspend(float seconds);
  void update();

  static ScriptInterpreter* current()
  {
    return _current;
  }

private:
  HSQUIRRELVM v;
  static ScriptInterpreter* _current;
  Timer resume_timer;
};

#endif

