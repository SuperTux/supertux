/*
  setup.h
  
  Super Tux - Setup
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - April 13, 2000
*/

void st_setup(void);
void st_shutdown(void);
void st_abort(char * reason, char * details);
void parseargs(int argc, char * argv[]);
