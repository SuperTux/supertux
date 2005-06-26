#ifndef __SCRIPT_INTERPRETER_H__
#define __SCRIPT_INTERPRETER_H__

#include <squirrel.h>
#include <iostream>
#include "timer.hpp"
#include "game_object.hpp"
#include "scripting/sound.hpp"
#include "scripting/level.hpp"

class Sector;

class ScriptInterpreter : public GameObject
{
public:
  ScriptInterpreter(const std::string& working_dir);
  ~ScriptInterpreter();

  void register_sector(Sector* sector);

  void draw(DrawingContext& );
  void update(float );

  void run_script(std::istream& in, const std::string& sourcename = "",
          bool remove_when_terminated = true);
  
  void expose_object(void* object, const std::string& name,
                     const std::string& type);

  void set_wakeup_time(float seconds);

  /** helper function that parses a script, starts it and adds it to the sector
   * specified
   */
  static void add_script_object(Sector* sector, const std::string& scriptname,
      const std::string& script);

  static ScriptInterpreter* current()
  {
    return _current;
  }

  const std::string& get_working_directory() const
  {
      return working_directory;
  }

private:
  HSQUIRRELVM v;
  static ScriptInterpreter* _current;
  Timer wakeup_timer;

  /// this directory is used as base for all filenames used in scripts
  std::string working_directory;
  Scripting::Sound* sound;
  Scripting::Level* level;
};

#endif

