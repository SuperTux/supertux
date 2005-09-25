#include <config.h>

#include "script_interpreter.hpp"

#include <stdarg.h>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <sqstdio.h>
#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdsystem.h>
#include <sqstdmath.h>
#include <sqstdstring.h>

#include "wrapper.hpp"
#include "wrapper_util.hpp"
#include "sector.hpp"
#include "file_system.hpp"
#include "game_session.hpp"
#include "resources.hpp"
#include "physfs/physfs_stream.hpp"
#include "object/text_object.hpp"
#include "object/scripted_object.hpp"
#include "object/display_effect.hpp"
#include "scripting/sound.hpp"
#include "scripting/scripted_object.hpp"
#include "scripting/display_effect.hpp"
#include "scripting/squirrel_error.hpp"

static void printfunc(HSQUIRRELVM, const char* str, ...)
{
  va_list arglist;
  va_start(arglist, str);
  vprintf(str, arglist);
  va_end(arglist);
}

ScriptInterpreter* ScriptInterpreter::_current = 0;

ScriptInterpreter::ScriptInterpreter(const std::string& new_working_directory)
  : working_directory(new_working_directory), sound(0), level(0)
{
  v = sq_open(1024);
  if(v == 0)
    throw std::runtime_error("Couldn't initialize squirrel vm");

  // register default error handlers
  sqstd_seterrorhandlers(v);
  // register squirrel libs
  sq_pushroottable(v);
  if(sqstd_register_bloblib(v) < 0)
    throw Scripting::SquirrelError(v, "Couldn't register blob lib");
  if(sqstd_register_iolib(v) < 0)
    throw Scripting::SquirrelError(v, "Couldn't register io lib");
  if(sqstd_register_systemlib(v) < 0)
    throw Scripting::SquirrelError(v, "Couldn't register system lib");
  if(sqstd_register_mathlib(v) < 0)
    throw Scripting::SquirrelError(v, "Couldn't register math lib");
  if(sqstd_register_stringlib(v) < 0)
    throw Scripting::SquirrelError(v, "Couldn't register string lib");

  // register print function
  sq_setprintfunc(v, printfunc);
  
  // register supertux API
  Scripting::register_supertux_wrapper(v);

  // expose some "global" objects
  sound = new Scripting::Sound();
  expose_object(sound, "Sound");
  
  level = new Scripting::Level();
  expose_object(level, "Level");
}

void
ScriptInterpreter::register_sector(Sector* sector)
{
  // expose ScriptedObjects to the script
  for(Sector::GameObjects::iterator i = sector->gameobjects.begin();
      i != sector->gameobjects.end(); ++i) {
    GameObject* object = *i;
    Scripting::ScriptedObject* scripted_object
      = dynamic_cast<Scripting::ScriptedObject*> (object);
    if(!scripted_object)
      continue;
    
    expose_object(scripted_object, scripted_object->get_name());
  }
  
  TextObject* text_object = new TextObject();
  sector->add_object(text_object);
  Scripting::Text* text = static_cast<Scripting::Text*> (text_object);
  expose_object(text, "Text");
  
  DisplayEffect* display_effect = new DisplayEffect();
  sector->add_object(display_effect);
  Scripting::DisplayEffect* display_effect_api
    = static_cast<Scripting::DisplayEffect*> (display_effect);
  expose_object(display_effect_api, "DisplayEffect");
}

ScriptInterpreter::~ScriptInterpreter()
{
  sq_close(v);
  delete sound;
  delete level;
}

static SQInteger squirrel_read_char(SQUserPointer file)
{
  std::istream* in = reinterpret_cast<std::istream*> (file);
  char c = in->get();
  if(in->eof())
    return 0;    
  return c;
}

void
ScriptInterpreter::run_script(std::istream& in, const std::string& sourcename,
        bool remove_when_terminated)
{
  printf("Stackbefore:\n");
  print_squirrel_stack(v);
  if(sq_compile(v, squirrel_read_char, &in, sourcename.c_str(), true) < 0)
    throw Scripting::SquirrelError(v, "Couldn't parse script");
 
  _current = this;
  sq_push(v, -2);
  if(sq_call(v, 1, false) < 0)
    throw Scripting::SquirrelError(v, "Couldn't start script");
  _current = 0;
  if(sq_getvmstate(v) != SQ_VMSTATE_SUSPENDED) {
    if(remove_when_terminated) {
      remove_me();
    }
    printf("ended.\n");
    // remove closure from stack
    sq_pop(v, 1);
  }
  printf("After:\n");
  print_squirrel_stack(v);
}

void
ScriptInterpreter::set_wakeup_time(float seconds)
{
  wakeup_timer.start(seconds);
}

void
ScriptInterpreter::update(float )
{
  if(!wakeup_timer.check())
    return;
  
  _current = this;
  if(sq_wakeupvm(v, false, false) < 0)
    throw Scripting::SquirrelError(v, "Couldn't resume script");
  _current = 0;
  if(sq_getvmstate(v) != SQ_VMSTATE_SUSPENDED) {
    printf("script ended...\n");
    remove_me();
  }
}

void
ScriptInterpreter::draw(DrawingContext& )
{
}

void
ScriptInterpreter::add_script_object(Sector* sector, const std::string& name,
    const std::string& script)
{
  try {
    std::string workdir = GameSession::current()->get_working_directory();
    std::auto_ptr<ScriptInterpreter> interpreter(
		new ScriptInterpreter(workdir));
    interpreter->register_sector(sector);
    
    // load global default.nut file if it exists
    //TODO: Load all .nut files from that directory
    try {
      std::string filename = "data/script/default.nut";
      IFileStream in(filename);
      interpreter->run_script(in, filename, false);
    } catch(std::exception& e) {
      // nothing
    }

    // load world-specific default.nut file if it exists
    try {
      std::string filename = workdir + "/default.nut";
      IFileStream in(filename);
      interpreter->run_script(in, filename, false);
    } catch(std::exception& e) {
      // nothing
    }
	
    std::istringstream in(script);
    interpreter->run_script(in, name);
    sector->add_object(interpreter.release());
  } catch(std::exception& e) {
    std::cerr << "Couldn't start '" << name << "' script: " << e.what() << "\n";
  }
}

