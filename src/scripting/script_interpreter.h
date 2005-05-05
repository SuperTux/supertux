#ifndef __SCRIPT_INTERPRETER_H__
#define __SCRIPT_INTERPRETER_H__

#include <squirrel.h>
#include <iostream>

class ScriptInterpreter
{
public:
    ScriptInterpreter();
    ~ScriptInterpreter();

    void load_script(std::istream& in, const std::string& sourcename = "");
    void run_script();
    void resume_script();
    bool script_suspended();

private:
    HSQUIRRELVM v;
};

#endif

