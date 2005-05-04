#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "tree.h"
#include <iostream>

// parsing
extern CompilationUnit* unit;
extern bool search_down;
extern Namespace* search_namespace;
extern Namespace* current_namespace;

// config/output
extern std::istream* input;
extern std::string inputfile;
extern std::string modulename;
extern std::string selected_namespace;

#endif

