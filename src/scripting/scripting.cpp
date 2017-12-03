//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "scripting/scripting.hpp"

#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <cstring>
#include <stdarg.h>
#include <stdio.h>

#include "object/anchor_point.hpp"
#include "physfs/ifile_stream.hpp"
#include "scripting/squirrel_error.hpp"
#include "scripting/wrapper.hpp"
#include "scripting/wrapper.interface.hpp"
#include "squirrel_util.hpp"
#include "supertux/console.hpp"
#include "util/log.hpp"

#ifdef ENABLE_SQDBG
#  include "../../external/squirrel/sqdbg/sqrdbg.h"
namespace {
HSQREMOTEDBG debugger = NULL;
} // namespace
#endif

namespace {

#ifdef __clang__
__attribute__((__format__ (__printf__, 2, 0)))
#endif
void printfunc(HSQUIRRELVM, const char* fmt, ...)
{
  char buf[4096];
  char separator[] = "\n";
  va_list arglist;
  va_start(arglist, fmt);
  vsnprintf(buf, sizeof(buf), fmt, arglist);
  char* ptr = strtok(buf, separator);
  while(ptr != NULL)
  {
    ConsoleBuffer::output << "[SCRIPTING] " << ptr << std::endl;
    ptr = strtok(NULL, separator);
  }
  va_end(arglist);
}

} // namespace

namespace scripting {

HSQUIRRELVM global_vm = NULL;

Scripting::Scripting(bool enable_debugger)
{
  global_vm = sq_open(64);
  if(global_vm == NULL)
    throw std::runtime_error("Couldn't initialize squirrel vm");

  if(enable_debugger) {
#ifdef ENABLE_SQDBG
    sq_enabledebuginfo(global_vm, SQTrue);
    debugger = sq_rdbg_init(global_vm, 1234, SQFalse);
    if(debugger == NULL)
      throw SquirrelError(global_vm, "Couldn't initialize squirrel debugger");

    sq_enabledebuginfo(global_vm, SQTrue);
    log_info << "Waiting for debug client..." << std::endl;
    if(SQ_FAILED(sq_rdbg_waitforconnections(debugger)))
      throw SquirrelError(global_vm, "Waiting for debug clients failed");
    log_info << "debug client connected." << std::endl;
#endif
  }

  sq_pushroottable(global_vm);
  if(SQ_FAILED(sqstd_register_bloblib(global_vm)))
    throw SquirrelError(global_vm, "Couldn't register blob lib");
  if(SQ_FAILED(sqstd_register_mathlib(global_vm)))
    throw SquirrelError(global_vm, "Couldn't register math lib");
  if(SQ_FAILED(sqstd_register_stringlib(global_vm)))
    throw SquirrelError(global_vm, "Couldn't register string lib");

  Sqrat::DefaultVM::Set(global_vm);

  // remove rand and srand calls from sqstdmath, we'll provide our own
  scripting::delete_table_entry(global_vm, "srand");
  scripting::delete_table_entry(global_vm, "rand");

  register_global_constants(global_vm);
  register_global_functions(global_vm);
  register_scripting_classes(global_vm);

  // register supertux API
  //register_supertux_wrapper(global_vm);

  sq_pop(global_vm, 1);

  // register print function
  sq_setprintfunc(global_vm, printfunc, printfunc);
  // register default error handlers
  sqstd_seterrorhandlers(global_vm);

  // try to load default script
  try {
    std::string filename = "scripts/default.nut";
    IFileStream stream(filename);
    scripting::compile_and_run(global_vm, stream, filename);
  } catch(std::exception& e) {
    log_warning << "Couldn't load default.nut: " << e.what() << std::endl;
  }
}

Scripting::~Scripting()
{
#ifdef ENABLE_SQDBG
  if(debugger != NULL) {
    sq_rdbg_shutdown(debugger);
    debugger = NULL;
  }
#endif

  if (global_vm)
    sq_close(global_vm);

  global_vm = NULL;
}

void
Scripting::register_global_constants(HSQUIRRELVM vm)
{
  using namespace Sqrat;
  ConstTable(vm).Const("ANCHOR_H_MASK", AnchorPoint::ANCHOR_H_MASK);
  ConstTable(vm).Const("ANCHOR_TOP", AnchorPoint::ANCHOR_TOP);
  ConstTable(vm).Const("ANCHOR_BOTTOM", AnchorPoint::ANCHOR_BOTTOM);
  ConstTable(vm).Const("ANCHOR_V_MASK", AnchorPoint::ANCHOR_V_MASK);
  ConstTable(vm).Const("ANCHOR_LEFT", AnchorPoint::ANCHOR_LEFT);
  ConstTable(vm).Const("ANCHOR_RIGHT", AnchorPoint::ANCHOR_RIGHT);
  ConstTable(vm).Const("ANCHOR_MIDDLE", AnchorPoint::ANCHOR_MIDDLE);
  ConstTable(vm).Const("ANCHOR_TOP_LEFT", AnchorPoint::ANCHOR_TOP_LEFT);
  ConstTable(vm).Const("ANCHOR_TOP_RIGHT", AnchorPoint::ANCHOR_TOP_RIGHT);
  ConstTable(vm).Const("ANCHOR_BOTTOM_LEFT", AnchorPoint::ANCHOR_BOTTOM_LEFT);
  ConstTable(vm).Const("ANCHOR_BOTTOM_RIGHT", AnchorPoint::ANCHOR_BOTTOM_RIGHT);
}

void
Scripting::register_global_functions(HSQUIRRELVM vm)
{
  using namespace Sqrat;
  RootTable(vm).Func("is_christmas", &is_christmas_as_bool);
  RootTable(vm).Func("exit_screen", &exit_screen);
  RootTable(vm).Func("fadeout_screen", &fadeout_screen);
  RootTable(vm).Func("shrink_screen", &shrink_screen);
  RootTable(vm).Func("abort_screenfade", &abort_screenfade);
  RootTable(vm).Func("translate", &translate);
  RootTable(vm).Func("_", &_);
  RootTable(vm).Func("display_text_file", &display_text_file);
  RootTable(vm).Func("load_worldmap", &load_worldmap);
  RootTable(vm).Func("set_next_worldmap", &set_next_worldmap);
  RootTable(vm).Func("load_level", &load_level);
  RootTable(vm).Func("import", &import_script);
  RootTable(vm).Func("debug_collrects", &debug_collrects);
  RootTable(vm).Func("debug_show_fps", &debug_show_fps);
  RootTable(vm).Func("debug_draw_solids_only", &debug_draw_solids_only);
  RootTable(vm).Func("debug_draw_editor_images", &debug_draw_editor_images);
  RootTable(vm).Func("debug_worldmap_ghost", &debug_worldmap_ghost);
  RootTable(vm).Func("save_state", &save_state);
  RootTable(vm).Func("load_state", &load_state);
  RootTable(vm).Func("play_music", &play_music);
  RootTable(vm).Func("play_sound", &play_sound);
  RootTable(vm).Func("grease", &grease);
  RootTable(vm).Func("invincible", &invincible);
  RootTable(vm).Func("ghost", &ghost);
  RootTable(vm).Func("mortal", &mortal);
  RootTable(vm).Func("restart", &restart);
  RootTable(vm).Func("whereami", &whereami);
  RootTable(vm).Func("gotoend", &gotoend);
  RootTable(vm).Func("warp", &warp);
  RootTable(vm).Func("camera", &camera);
  RootTable(vm).Func("set_gamma", &set_gamma);
  RootTable(vm).Func("quit", &quit);
  RootTable(vm).Func("rand", &rand);
  RootTable(vm).Func("set_game_speed", &set_game_speed);
  RootTable(vm).Func("record_demo", &record_demo);
  RootTable(vm).Func("play_demo", &play_demo);

  RootTable(vm).Func("Level_finish", &Level_finish);
  RootTable(vm).Func("Level_spawn", &Level_spawn);
  RootTable(vm).Func("Level_flip_vertically", &Level_flip_vertically);
  RootTable(vm).Func("Level_toggle_pause", &Level_toggle_pause);
  RootTable(vm).Func("Level_edit", &Level_edit);
}

void
Scripting::register_scripting_classes(HSQUIRRELVM vm)
{
  register_scripting_class<scripting::AmbientSound>(vm, "AmbientSound");
  register_scripting_class<scripting::Background>(vm, "Background");
  register_scripting_class<scripting::Camera>(vm, "Camera");
  register_scripting_class<scripting::Candle>(vm, "Candle");
  register_scripting_class<scripting::DisplayEffect>(vm, "DisplayEffect");
  register_scripting_class<scripting::FloatingImage>(vm, "FloatingImage");
  register_scripting_class<scripting::Gradient>(vm, "Gradient");
  register_scripting_class<scripting::LevelTime>(vm, "LevelTime");
  register_scripting_class<scripting::ParticleSystem>(vm, "ParticleSystem");
  register_scripting_class<scripting::Platform>(vm, "Platform");
  register_scripting_class<scripting::Player>(vm, "Player");
  register_scripting_class<scripting::ScriptedObject>(vm, "ScriptedObject");
  register_scripting_class<scripting::Sector>(vm, "Sector");
  register_scripting_class<scripting::Text>(vm, "Text");
  register_scripting_class<scripting::Thunderstorm>(vm, "Thunderstorm");
  register_scripting_class<scripting::TileMap>(vm, "TileMap");
  register_scripting_class<scripting::WillOWisp>(vm, "WillOWisp");
  register_scripting_class<scripting::Wind>(vm, "Wind");
}

template<class T>
void
Scripting::register_scripting_class(HSQUIRRELVM v, const std::string& name)
{
  using namespace Sqrat;
  Class<T, NoCopy<T>> sqratClass(v, name.c_str());
  // Check whether the scripting class implements
  // register_exposed_methods. Not beautiful, but
  // it gets the job done.
  if(std::is_base_of<SQRatObject<T>, T>::value)
  {
    SQRatObject<T>::register_exposed_methods(v, sqratClass);
  }
  RootTable(v).Bind(name.c_str(), sqratClass);
}

void
Scripting::update_debugger()
{
#ifdef ENABLE_SQDBG
  if(debugger != NULL)
    sq_rdbg_update(debugger);
#endif
}

} // namespace scripting

/* EOF */
