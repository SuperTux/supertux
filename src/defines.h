/*
  defines.h
  
  Super Tux
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/

  April 11, 2000 - August 29, 2002
*/


#if !defined( SUPERTUX_DEFINES_H )

        #define SUPERTUX_DEFINES_H
        /* Version: */

        #define VERSION "0.0.5"


        /* Frames per second: */

        #define FPS (1000 / 25)


        /* Joystick buttons and axes: */

        #define JOY_A 0
        #define JOY_B 1

        #define JOY_X 0
        #define JOY_Y 1


        /* Booleans: */

        #define NO 0
        #define YES 1

        /* Debugging */

        #ifdef DEBUG
        	#define DEBUG_MSG( msg ) { \
                	printf( msg ); printf("\n"); \
                }
        #else
        	#define DEBUG_MSG( msg ) {}
        #endif

#endif

