#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "tree.hpp"
#include <iostream>

// parsing
extern CompilationUnit* unit;
extern bool search_down;
extern Namespace* search_namespace;
extern Namespace* current_namespace;
extern std::string last_docucomment;
// the first file indicated by # 1 "..."
// (this is what the C preprocessor outputs so that you know which was the
// original file before preprocessing
extern std::string original_file;
// the filename where the current fragment came from (before it was included by
// the preprocessor)
extern std::string current_file;
// get line number inside the current_file
int getCurrentLine();

// config/output
extern std::istream* input;
extern std::string inputfile;
extern std::string modulename;
extern std::string selected_namespace;

#endif
