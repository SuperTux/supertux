#include <stdio.h>
#include "functions.h"
#include "script_interpreter.h"

namespace Scripting
{

void wait(float seconds)
{
  ScriptInterpreter::current()->suspend(seconds);
}

}

