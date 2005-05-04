#ifndef __SCRIPT_INTERPRETER_H__
#define __SCRIPT_INTERPRETER_H__

class ScriptInterpreter
{
public:
    ScriptInterpreter();
    ~ScriptInterpreter();

    void load_script(const std::istream& in);
    void run_script();
    void resume_script();
    bool script_suspended() const;
};

#endif

