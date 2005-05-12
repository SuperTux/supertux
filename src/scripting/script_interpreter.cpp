#include <config.h>

#include "script_interpreter.h"

#include <stdarg.h>
#include <stdexcept>
#include <sstream>
#include <sqstdio.h>
#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdsystem.h>
#include <sqstdmath.h>
#include <sqstdstring.h>

#include "wrapper.h"
#include "wrapper_util.h"
#include "sector.h"
#include "file_system.h"
#include "game_session.h"
#include "object/text_object.h"
#include "object/scripted_object.h"
#include "object/display_effect.h"
#include "scripting/sound.h"
#include "scripting/scripted_object.h"
#include "scripting/display_effect.h"

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
    throw SquirrelError(v, "Couldn't register blob lib");
  if(sqstd_register_iolib(v) < 0)
    throw SquirrelError(v, "Couldn't register io lib");
  if(sqstd_register_systemlib(v) < 0)
    throw SquirrelError(v, "Couldn't register system lib");
  if(sqstd_register_mathlib(v) < 0)
    throw SquirrelError(v, "Couldn't register math lib");
  if(sqstd_register_stringlib(v) < 0)
    throw SquirrelError(v, "Couldn't register string lib");

  // register print function
  sq_setprintfunc(v, printfunc);
  
  // register supertux API
  register_functions(v, supertux_global_functions);
  register_classes(v, supertux_classes);

  // expose some "global" objects
  sound = new Scripting::Sound();
  expose_object(sound, "Sound", "Sound");  
  
  level = new Scripting::Level();
  expose_object(level, "Level", "Level");
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
    
    expose_object(scripted_object, scripted_object->get_name(), 
        "ScriptedObject");
  }
  
  TextObject* text_object = new TextObject();
  sector->add_object(text_object);
  Scripting::Text* text = static_cast<Scripting::Text*> (text_object);
  expose_object(text, "Text", "Text");
  
  DisplayEffect* display_effect = new DisplayEffect();
  sector->add_object(display_effect);
  Scripting::DisplayEffect* display_effect_api
    = static_cast<Scripting::DisplayEffect*> (display_effect);
  expose_object(display_effect_api, "DisplayEffect", "DisplayEffect");
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
ScriptInterpreter::load_script(std::istream& in, const std::string& sourcename)
{
  if(sq_compile(v, squirrel_read_char, &in, sourcename.c_str(), true) < 0)
    throw SquirrelError(v, "Couldn't parse script");
}

void
ScriptInterpreter::start_script()
{
  _current = this;
  sq_push(v, -2);
  if(sq_call(v, 1, false) < 0)
    throw SquirrelError(v, "Couldn't start script");
  _current = 0;
  if(sq_getvmstate(v) != SQ_VMSTATE_SUSPENDED) {
    printf("script ended...\n");
    remove_me();
  }  
}

void
ScriptInterpreter::expose_object(void* object, const std::string& name,
                                 const std::string& type)
{
  // part1 of registration of the instance in the root table
  sq_pushroottable(v);
  sq_pushstring(v, name.c_str(), -1);

  // resolve class name
  sq_pushroottable(v);
  sq_pushstring(v, type.c_str(), -1);
  if(sq_get(v, -2) < 0) {
    std::ostringstream msg;
    msg << "Couldn't resolve squirrel type '" << type << "'.";
    throw std::runtime_error(msg.str());
  }
  sq_remove(v, -2); // remove roottable
  
  // create an instance and set pointer to c++ object
  if(sq_createinstance(v, -1) < 0 || sq_setinstanceup(v, -1, object)) {
    std::ostringstream msg;
    msg << "Couldn't setup squirrel instance for object '"
        << name << "' of type '" << type << "'.";
    throw SquirrelError(v, msg.str());
  }
  
  sq_remove(v, -2); // remove class from stack
  
  // part2 of registration of the instance in the root table
  if(sq_createslot(v, -3) < 0)
    throw SquirrelError(v, "Couldn't register object in squirrel root table");    sq_pop(v, 1);
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
    throw SquirrelError(v, "Couldn't resume script");
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
